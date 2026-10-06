#ifndef RENDERER_SHADER_INCLUDE
#define RENDERER_SHADER_INCLUDE
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <vector>

#include "Math.h"
#include "Mesh.h"
#include "Platform/PixelFormat.h"

struct UniformBuffer
{
    Mat4 view = Mat4::Identity();
    Mat4 projection = Mat4::Identity();
    Vec4f cameraPos;
    Vec4f lightDir;
    Vec4f lightColor;
};

class Material;

class Shader
{
protected:
    // 顶点阶段输出缓冲：Vertex 写入、光栅化阶段读取，
    // 仅作为 Shader 内部顶点数据传递的媒介，不对外暴露
    struct PixelBuffer
    {
        Vec4f* screenVerts = nullptr;
        Vec4f* viewNormals = nullptr;
        Vec4f* uvs = nullptr;
        // 每顶点世界空间坐标，PBR 等待色模型需要用它算视线方向
        Vec4f* worldPositions = nullptr;
        // 每顶点裁剪空间 w 的倒数 (1/w_clip)，光栅化阶段透视矫正插值用
        float* invWs = nullptr;
        int vertexCount = 0;

        PixelBuffer() = default;

        ~PixelBuffer()
        {
            Release();
        }

        PixelBuffer(const PixelBuffer&) = delete;
        PixelBuffer& operator=(const PixelBuffer&) = delete;

        void Allocate(int count)
        {
            if (vertexCount != count)
            {
                Release();
                screenVerts = new Vec4f[count];
                viewNormals = new Vec4f[count];
                uvs = new Vec4f[count];
                worldPositions = new Vec4f[count];
                invWs = new float[count];
                vertexCount = count;
            }
        }

        void Release()
        {
            delete[] screenVerts;
            delete[] viewNormals;
            delete[] uvs;
            delete[] worldPositions;
            delete[] invWs;
            screenVerts = nullptr;
            viewNormals = nullptr;
            uvs = nullptr;
            worldPositions = nullptr;
            invWs = nullptr;
            vertexCount = 0;
        }
    };

    // 光栅化阶段输出缓冲：Rasterize 写入逐像素插值属性与覆盖标记，
    // Pixel 阶段读取并着色，仅作为 Shader 内部像素数据传递的媒介
    struct FragmentBuffer
    {
        Vec4f* normals = nullptr;
        Vec4f* uvs = nullptr;
        // 逐像素世界空间坐标（透视矫正插值结果）
        Vec4f* worldPositions = nullptr;
        // 0 = 未通过覆盖/深度测试，1 = 待着色
        std::uint8_t* coverage = nullptr;
        int pixelCount = 0;

        FragmentBuffer() = default;

        ~FragmentBuffer()
        {
            Release();
        }

        FragmentBuffer(const FragmentBuffer&) = delete;
        FragmentBuffer& operator=(const FragmentBuffer&) = delete;

        void Allocate(int count)
        {
            if (pixelCount != count)
            {
                Release();
                normals = new Vec4f[count];
                uvs = new Vec4f[count];
                worldPositions = new Vec4f[count];
                coverage = new std::uint8_t[count];
                pixelCount = count;
            }
        }

        void Release()
        {
            delete[] normals;
            delete[] uvs;
            delete[] worldPositions;
            delete[] coverage;
            normals = nullptr;
            uvs = nullptr;
            worldPositions = nullptr;
            coverage = nullptr;
            pixelCount = 0;
        }
    };

    inline static UniformBuffer uniforms;
    PixelBuffer pixelBuffer;
    FragmentBuffer fragmentBuffer;

    // 所属材质，供着色阶段采样贴图；由 Material 绑定时写入
    const Material* material = nullptr;

public:
    static void SetUniforms(const UniformBuffer& ub) { uniforms = ub; }

    void SetMaterial(const Material* m) { material = m; }

    Shader(){};
    virtual ~Shader(){};
    void Render(Mesh& mesh, int width, int height, PixelFormat format,
                float* depthBuffer, std::uint32_t* colorBuffer)
    {
        int totalPixels = width * height;
        std::fill(depthBuffer, depthBuffer + totalPixels, 1.0f);
        std::fill(colorBuffer, colorBuffer + totalPixels, (std::uint32_t)0);

        Vertex(mesh, width, height);
        Rasterize(width, height, depthBuffer);
        PixelStage(width, height, format, colorBuffer);
    }

protected:
    virtual void Vertex(Mesh& mesh, int width, int height) = 0;
    virtual std::uint32_t Pixel(std::uint32_t& pixel, PixelFormat format, const Vec4f& worldPos,
                                const Vec4f& normal, const Vec4f& uv) = 0;

    // 光栅化：三角形外层 + 包围盒裁剪。
    // 先预计算每个三角形的面积（顺带背面剔除）与屏幕包围盒，
    // 然后每个三角形只测试其包围盒内的像素，命中后用透视矫正的
    // 重心坐标插值 uv、法线等属性，结果写入 FragmentBuffer 供 Pixel 阶段使用。
    // 复杂度从 O(宽 × 高 × 三角形数) 降到 O(Σ 三角形覆盖面积)。
    //
    // 正确性说明：
    // 1) 三角形内任一点是三顶点的凸组合，必然落在顶点 min/max 围成的包围盒内，
    //    包围盒外的覆盖测试必然失败，裁剪不损失正确性；
    // 2) 三角形遍历顺序不影响结果——同一像素被多个三角形覆盖时，
    //    每次写入都经过深度测试，最终保留最近者。
    //
    // 串行执行：不同三角形可能写同一像素，若需并行应按屏幕 tile 分块（后续优化方向）。
    void Rasterize(int width, int height, float* depthBuffer)
    {
        int totalPixels = width * height;
        fragmentBuffer.Allocate(totalPixels);
        std::fill(fragmentBuffer.coverage, fragmentBuffer.coverage + totalPixels, (std::uint8_t)0);

        BuildTriangleCache(width, height);

        for (const CachedTriangle& tri : triangles)
        {
            const Vec4f& a = pixelBuffer.screenVerts[tri.i0];
            const Vec4f& b = pixelBuffer.screenVerts[tri.i1];
            const Vec4f& c = pixelBuffer.screenVerts[tri.i2];

            for (int y = tri.minY; y <= tri.maxY; y++)
            {
                for (int x = tri.minX; x <= tri.maxX; x++)
                {
                    Vec4f p((float)x, (float)y, 0.0f, 0.0f);

                    // 重心坐标：area 已预计算（取倒数，乘法代替除法），
                    // 每个候选像素只需两次子面积计算，负值提前跳出
                    float u = Math::SignedTriangleArea(b, c, p) * tri.invArea;
                    if (u < 0.0f) continue;
                    float v = Math::SignedTriangleArea(c, a, p) * tri.invArea;
                    if (v < 0.0f) continue;
                    float w = 1.0f - u - v;
                    if (w < 0.0f) continue;

                    int idx = y * width + x;

                    // NDC 深度在屏幕空间是线性的，直接用原始重心坐标插值
                    float depth = a.z * u + b.z * v + c.z * w;

                    if (depth > depthBuffer[idx]) continue;
                    depthBuffer[idx] = depth;

                    // 透视矫正：屏幕空间重心坐标需除以各自顶点的 w_clip 再归一化，
                    // 即 w_i' = bary_i * (1/w_i)，w_i'' = w_i' / Σw'
                    float w0 = u * pixelBuffer.invWs[tri.i0];
                    float w1 = v * pixelBuffer.invWs[tri.i1];
                    float w2 = w * pixelBuffer.invWs[tri.i2];
                    float invSum = 1.0f / (w0 + w1 + w2);
                    w0 *= invSum;
                    w1 *= invSum;
                    w2 *= invSum;

                    fragmentBuffer.normals[idx] = pixelBuffer.viewNormals[tri.i0] * w0
                                                + pixelBuffer.viewNormals[tri.i1] * w1
                                                + pixelBuffer.viewNormals[tri.i2] * w2;
                    fragmentBuffer.uvs[idx] = pixelBuffer.uvs[tri.i0] * w0
                                            + pixelBuffer.uvs[tri.i1] * w1
                                            + pixelBuffer.uvs[tri.i2] * w2;
                    fragmentBuffer.worldPositions[idx] = pixelBuffer.worldPositions[tri.i0] * w0
                                                       + pixelBuffer.worldPositions[tri.i1] * w1
                                                       + pixelBuffer.worldPositions[tri.i2] * w2;
                    fragmentBuffer.coverage[idx] = 1;
                }
            }
        }
    }

    // 像素阶段：遍历 Rasterize 输出的覆盖像素，逐像素调用 Pixel 着色
    void PixelStage(int width, int height, PixelFormat format, std::uint32_t* colorBuffer)
    {
        int totalPixels = width * height;
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
        for (int idx = 0; idx < totalPixels; idx++)
        {
            if (!fragmentBuffer.coverage[idx]) continue;

            std::uint32_t color = Pixel(colorBuffer[idx], format,
                                        fragmentBuffer.worldPositions[idx],
                                        fragmentBuffer.normals[idx],
                                        fragmentBuffer.uvs[idx]);
            colorBuffer[idx] = color;
        }
    }

private:
    // 三角形预计算缓存：面积倒数与屏幕包围盒。
    // 这些数据与像素无关，每帧每三角形只算一次，
    // 避免在像素内层循环里对同一三角形重复求面积、重复做背面剔除
    struct CachedTriangle
    {
        int i0, i1, i2;    // 顶点索引（指向 pixelBuffer）
        float invArea;     // 1 / 屏幕空间有符号面积
        int minX, minY;    // 屏幕包围盒（已 clamp 到窗口内，闭区间）
        int maxX, maxY;
    };

    std::vector<CachedTriangle> triangles;

    // 预计算：背面剔除（area <= 0 含退化三角形）、求面积倒数、算包围盒。
    // 顶点被裁剪（w <= 0）的三角形被顶点着色器推送到 (-1e6, -1e6)，
    // 包围盒 clamp 后必为空区间，在这里一并剔除
    void BuildTriangleCache(int width, int height)
    {
        triangles.clear();
        int triCount = pixelBuffer.vertexCount / 3;

        for (int t = 0; t < triCount; t++)
        {
            int i0 = t * 3 + 0;
            int i1 = t * 3 + 1;
            int i2 = t * 3 + 2;

            const Vec4f& a = pixelBuffer.screenVerts[i0];
            const Vec4f& b = pixelBuffer.screenVerts[i1];
            const Vec4f& c = pixelBuffer.screenVerts[i2];

            float area = Math::SignedTriangleArea(a, b, c);
            if (area <= 0.0f) continue;

            int minX = std::max(0,           (int)std::floor(std::min({a.x, b.x, c.x})));
            int maxX = std::min(width  - 1,  (int)std::ceil(std::max({a.x, b.x, c.x})));
            int minY = std::max(0,           (int)std::floor(std::min({a.y, b.y, c.y})));
            int maxY = std::min(height - 1,  (int)std::ceil(std::max({a.y, b.y, c.y})));
            if (minX > maxX || minY > maxY) continue;

            CachedTriangle tri;
            tri.i0 = i0;
            tri.i1 = i1;
            tri.i2 = i2;
            tri.invArea = 1.0f / area;
            tri.minX = minX;
            tri.minY = minY;
            tri.maxX = maxX;
            tri.maxY = maxY;
            triangles.push_back(tri);
        }
    }
};
#endif
