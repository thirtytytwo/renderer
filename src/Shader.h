#ifndef RENDERER_SHADER_INCLUDE
#define RENDERER_SHADER_INCLUDE
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_pixels.h>

#include <vector>

#include "Math.h"
#include "Mesh.h"

class Shader
{
protected:
    std::vector<Vec4> screenVerts;

public:
    Shader(){};
    ~Shader(){};
    void Render(Uint32* pixelData, Mesh& mesh, const int width, const int height, const SDL_PixelFormat format)
    {
        Vertex(mesh, width, height);

        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                if(!DoBarycentric(Vec4((float)x, (float)y, 0, 0))) continue;

                Uint32 color = Pixel(pixelData[y * width + x], format);
                pixelData[y*width+x] = color;
            }
        }
    }

protected:
    virtual void Vertex(Mesh& mesh, int width, int height) = 0;
    virtual Uint32 Pixel(Uint32& pixel, const SDL_PixelFormat format) = 0;

private:
    bool DoBarycentric(const Vec4& pixel)
    {
        int triCount = screenVerts.size() / 3;

        for (int t = 0; t < triCount; t++)
        {
            const Vec4& a = screenVerts[t * 3 + 0];
            const Vec4& b = screenVerts[t * 3 + 1];
            const Vec4& c = screenVerts[t * 3 + 2];

            float area = Math::SignedTriangleArea(a, b, c);
            if (area == 0.0f) continue;

            float u = Math::SignedTriangleArea(b, c, pixel) / area;
            float v = Math::SignedTriangleArea(c, a, pixel) / area;
            float w = 1.0f - u - v;

            if (u >= 0.0f && v >= 0.0f && w >= 0.0f)
                return true;
        }

        return false;
    }
};
#endif