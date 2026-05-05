#ifndef RENDERER_SIMPLE_SHADER_INCLUDE
#define RENDERER_SIMPLE_SHADER_INCLUDE

#include "Shader.h"

class SimpleShader : public Shader
{
protected:
    void Vertex(Mesh& mesh, int width, int height) override
    {
        float fov = Math::PI / 3.0f;
        float aspect = (float)width / (float)height;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;

        Vec4f eye(0.0f, 0.0f, 5.0f, 1.0f);
        Vec4f target(0.0f, 0.0f, 0.0f, 1.0f);
        Vec4f up(0.0f, 1.0f, 0.0f, 0.0f);

        Mat4 model = Mat4::Identity();
        Mat4 view = Mat4::LookAt(eye, target, up);
        Mat4 projection = Mat4::Perspective(fov, aspect, nearPlane, farPlane);
        Mat4 mvp = projection * view * model;

        int vertexCount = mesh.triangleCount * 3;
        screenVerts.resize(vertexCount);

        for (int i = 0; i < vertexCount; i++)
        {
            Vec4f pos(mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z, 1.0f);

            Vec4f clipPos = mvp * pos;

            if (clipPos.w <= 0.0f)
            {
                screenVerts[i] = Vec4f(-1e6f, -1e6f, 0, 0);
                continue;
            }

            Vec4f ndcPos = clipPos / Vec4f(clipPos.w);

            float screenX = (ndcPos.x + 1.0f) * 0.5f * width;
            float screenY = (1.0f - ndcPos.y) * 0.5f * height;

            screenVerts[i] = Vec4f(screenX, screenY, ndcPos.z, 1.0f);
        }
    }

    Uint32 Pixel(Uint32& pixel, const SDL_PixelFormat format) override
    {
        Uint32 color = SDL_MapRGB(SDL_GetPixelFormatDetails(format), 0,
                                    (Uint8)255, (Uint8)255, (Uint8)255);
        return color;
    }
};

#endif