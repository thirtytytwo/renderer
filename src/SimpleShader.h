#ifndef RENDERER_SIMPLE_SHADER_INCLUDE
#define RENDERER_SIMPLE_SHADER_INCLUDE

#include <cstdint>

#include "Material.h"
#include "Shader.h"

class SimpleShader : public Shader
{
protected:
    void Vertex(Mesh& mesh, int width, int height) override
    {
        Mat4 model = Mat4::Identity();
        Mat4 mvp = uniforms.projection * uniforms.view * model;

        int vertexCount = mesh.triangleCount * 3;
        pixelBuffer.Allocate(vertexCount);

        for (int i = 0; i < vertexCount; i++)
        {
            Vec4f pos(mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z, 1.0f);

            Vec4f clipPos = mvp * pos;

            if (clipPos.w <= 0.0f)
            {
                pixelBuffer.screenVerts[i] = Vec4f(-1e6f, -1e6f, 0, 0);
                pixelBuffer.viewNormals[i] = Vec4f(0, 0, 0, 0);
                pixelBuffer.uvs[i] = Vec4f(0, 0, 0, 0);
                pixelBuffer.worldPositions[i] = Vec4f(0, 0, 0, 0);
                pixelBuffer.invWs[i] = 0.0f;
                continue;
            }

            Vec4f ndcPos = clipPos / Vec4f(clipPos.w);

            float screenX = (ndcPos.x + 1.0f) * 0.5f * width;
            // 保持 Y 向上（数学坐标系），使屏幕空间三角形绕序与 DoBarycentric
            // 中的 area > 0 背面剔除判据一致。上下翻转在呈现阶段统一处理。
            float screenY = (ndcPos.y + 1.0f) * 0.5f * height;

            pixelBuffer.screenVerts[i] = Vec4f(screenX, screenY, ndcPos.z, 1.0f);
            // 保存 1/w_clip，光栅化阶段透视矫正插值用
            pixelBuffer.invWs[i] = 1.0f / clipPos.w;

            Vec4f worldNormal = model * mesh.normals[i];
            worldNormal.w = 0.0f;
            pixelBuffer.viewNormals[i] = worldNormal.normalized();

            // 世界空间坐标，光栅化阶段透视矫正插值后供像素着色使用
            Vec4f worldPos = model * pos;
            pixelBuffer.worldPositions[i] = worldPos;

            pixelBuffer.uvs[i] = mesh.uvs[i];
        }
    }

    std::uint32_t Pixel(std::uint32_t& pixel, PixelFormat format, const Vec4f& worldPos,
                        const Vec4f& normal, const Vec4f& uv) override
    {
        (void)worldPos; // 兰伯特模型不需要位置信息
        Vec4f n = normal.normalized();
        float NDotL = n.dot(-uniforms.lightDir);

        // 采样主色贴图，叠加兰伯特光照
        Vec4f albedo = material->Sample(uv.x, uv.y);
        Vec4f color = albedo * NDotL;

        return PackColorF(format, color.x, color.y, color.z, albedo.w);
    }
};

#endif
