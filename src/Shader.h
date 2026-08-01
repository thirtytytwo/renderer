#ifndef RENDERER_SHADER_INCLUDE
#define RENDERER_SHADER_INCLUDE
#include <SDL3/SDL_stdinc.h>
#include <algorithm>

#include "Math.h"
#include "Mesh.h"
#include "PixelBuffer.h"
#include "Render/RenderResources.h"

struct UniformBuffer
{
    Mat4 view = Mat4::Identity();
    Mat4 projection = Mat4::Identity();
    Vec4f cameraPos;
    Vec4f lightDir;
    Vec4f lightColor;
};

class Shader
{
protected:
    inline static UniformBuffer uniforms;
    PixelBuffer pixelBuffer;

public:
    static void SetUniforms(const UniformBuffer& ub) { uniforms = ub; }

    Shader(){};
    ~Shader(){};
    void Render(Mesh& mesh, const RenderContext& ctx)
    {
        int totalPixels = ctx.width * ctx.height;
        std::fill(ctx.depth, ctx.depth + totalPixels, 1.0f);
        std::fill(ctx.color, ctx.color + totalPixels, (Uint32)0);

        Vertex(mesh, ctx);

        for(int y = 0; y < ctx.height; y++)
        {
            for(int x = 0; x < ctx.width; x++)
            {
                int idx = y * ctx.width + x;
                Vec4f interpNormal;
                float depth;
                if(!DoBarycentric(Vec4f((float)x, (float)y, 0, 0), interpNormal, depth)) continue;

                if(depth > ctx.depth[idx]) continue;
                ctx.depth[idx] = depth;

                Uint32 color = Pixel(ctx.color[idx], ctx.format, interpNormal);
                ctx.color[idx] = color;
            }
        }
    }

protected:
    virtual void Vertex(Mesh& mesh, const RenderContext& ctx) = 0;
    virtual Uint32 Pixel(Uint32& pixel, const SDL_PixelFormat format, const Vec4f& normal) = 0;

private:
    bool DoBarycentric(const Vec4f& pixel, Vec4f& outNormal, float& outDepth)
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
                outNormal = pixelBuffer.viewNormals[i0] * u
                          + pixelBuffer.viewNormals[i1] * v
                          + pixelBuffer.viewNormals[i2] * w;
                outDepth = a.z * u + b.z * v + c.z * w;
                return true;
            }
        }

        return false;
    }
};
#endif
