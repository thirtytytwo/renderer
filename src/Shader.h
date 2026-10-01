#ifndef RENDERER_SHADER_INCLUDE
#define RENDERER_SHADER_INCLUDE
#include <cstdint>
#include <algorithm>

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
                invWs = new float[count];
                vertexCount = count;
            }
        }

        void Release()
        {
            delete[] screenVerts;
            delete[] viewNormals;
            delete[] uvs;
            delete[] invWs;
            screenVerts = nullptr;
            viewNormals = nullptr;
            uvs = nullptr;
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
                coverage = new std::uint8_t[count];
                pixelCount = count;
            }
        }

        void Release()
        {
            delete[] normals;
            delete[] uvs;
            delete[] coverage;
            normals = nullptr;
            uvs = nullptr;
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
    virtual std::uint32_t Pixel(std::uint32_t& pixel, PixelFormat format, const Vec4f& normal, const Vec4f& uv) = 0;

    // 光栅化：逐像素做三角形覆盖测试与深度测试，命中后用透视矫正的
    // 重心坐标插值 uv、法线等属性，结果写入 FragmentBuffer 供 Pixel 阶段使用
    void Rasterize(int width, int height, float* depthBuffer)
    {
        int totalPixels = width * height;
        fragmentBuffer.Allocate(totalPixels);
        std::fill(fragmentBuffer.coverage, fragmentBuffer.coverage + totalPixels, (std::uint8_t)0);

        // 分块并行：外层 y 循环按连续行块分配给各线程，
        // 内层 x 循环在每个线程内串行执行。
        // schedule(static) 默认均分连续块，各线程写入互不重叠的行，无数据竞争
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                int idx = y * width + x;
                Vec4f bary;
                int triIndex;
                if(!DoBarycentric(Vec4f((float)x, (float)y, 0, 0), bary, triIndex)) continue;

                int i0 = triIndex * 3 + 0;
                int i1 = triIndex * 3 + 1;
                int i2 = triIndex * 3 + 2;

                // NDC 深度在屏幕空间是线性的，直接用原始重心坐标插值
                float depth = pixelBuffer.screenVerts[i0].z * bary.x
                            + pixelBuffer.screenVerts[i1].z * bary.y
                            + pixelBuffer.screenVerts[i2].z * bary.z;

                if(depth > depthBuffer[idx]) continue;
                depthBuffer[idx] = depth;

                // 透视矫正：屏幕空间重心坐标需除以各自顶点的 w_clip 再归一化，
                // 即 w_i' = bary_i * (1/w_i)，w_i'' = w_i' / Σw'
                float w0 = bary.x * pixelBuffer.invWs[i0];
                float w1 = bary.y * pixelBuffer.invWs[i1];
                float w2 = bary.z * pixelBuffer.invWs[i2];
                float invSum = 1.0f / (w0 + w1 + w2);
                w0 *= invSum;
                w1 *= invSum;
                w2 *= invSum;

                fragmentBuffer.normals[idx] = pixelBuffer.viewNormals[i0] * w0
                                            + pixelBuffer.viewNormals[i1] * w1
                                            + pixelBuffer.viewNormals[i2] * w2;
                fragmentBuffer.uvs[idx] = pixelBuffer.uvs[i0] * w0
                                        + pixelBuffer.uvs[i1] * w1
                                        + pixelBuffer.uvs[i2] * w2;
                fragmentBuffer.coverage[idx] = 1;
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
                                        fragmentBuffer.normals[idx],
                                        fragmentBuffer.uvs[idx]);
            colorBuffer[idx] = color;
        }
    }

private:
    // 仅做三角形覆盖测试（含 area>0 背面剔除），命中时输出原始
    // 屏幕空间重心坐标 (u, v, w) 和所属三角形索引，不做任何插值
    bool DoBarycentric(const Vec4f& pixel, Vec4f& outBary, int& outTriIndex)
    {
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

            float u = Math::SignedTriangleArea(b, c, pixel) / area;
            float v = Math::SignedTriangleArea(c, a, pixel) / area;
            float w = 1.0f - u - v;

            if (u >= 0.0f && v >= 0.0f && w >= 0.0f)
            {
                outBary = Vec4f(u, v, w, 0.0f);
                outTriIndex = t;
                return true;
            }
        }

        return false;
    }
};
#endif
