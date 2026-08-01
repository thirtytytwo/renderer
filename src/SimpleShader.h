#ifndef RENDERER_SIMPLE_SHADER_INCLUDE
#define RENDERER_SIMPLE_SHADER_INCLUDE

#include "Shader.h"

class SimpleShader : public Shader
{
protected:
    void Vertex(Mesh& mesh, const RenderContext& ctx) override
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
                continue;
            }

            Vec4f ndcPos = clipPos / Vec4f(clipPos.w);

            float screenX = (ndcPos.x + 1.0f) * 0.5f * ctx.width;
            float screenY = (ndcPos.y + 1.0f) * 0.5f * ctx.height;

            pixelBuffer.screenVerts[i] = Vec4f(screenX, screenY, ndcPos.z, 1.0f);

            Vec4f worldNormal = model * mesh.normals[i];
            worldNormal.w = 0.0f;
            pixelBuffer.viewNormals[i] = worldNormal.normalized();
        }
    }

    Uint32 Pixel(Uint32& pixel, const SDL_PixelFormat format, const Vec4f& normal) override
    {
        Vec4f n = normal.normalized();
        float NDotL = n.dot(-uniforms.lightDir);

        Vec4f color = Vec4f(NDotL, NDotL, NDotL, 1);

        return SDL_MapRGB(SDL_GetPixelFormatDetails(format), 0, (Uint8)(color.x * 255.f), (Uint8)(color.y * 255.f), (Uint8)(color.z * 255.f));
    }
};

#endif
