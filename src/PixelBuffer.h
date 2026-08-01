#ifndef RENDERER_PIXELBUFFER_INCLUDE
#define RENDERER_PIXELBUFFER_INCLUDE

#include "Math.h"

struct PixelBuffer
{
    Vec4f* screenVerts = nullptr;
    Vec4f* viewNormals = nullptr;
    int vertexCount = 0;

    PixelBuffer() = default;

    ~PixelBuffer()
    {
        Release();
    }

    PixelBuffer(const PixelBuffer&) = delete;
    PixelBuffer& operator=(const PixelBuffer&) = delete;

    PixelBuffer(PixelBuffer&& other) noexcept
        : screenVerts(other.screenVerts)
        , viewNormals(other.viewNormals)
        , vertexCount(other.vertexCount)
    {
        other.screenVerts = nullptr;
        other.viewNormals = nullptr;
        other.vertexCount = 0;
    }

    PixelBuffer& operator=(PixelBuffer&& other) noexcept
    {
        if (this != &other)
        {
            Release();
            screenVerts = other.screenVerts;
            viewNormals = other.viewNormals;
            vertexCount = other.vertexCount;
            other.screenVerts = nullptr;
            other.viewNormals = nullptr;
            other.vertexCount = 0;
        }
        return *this;
    }

    void Allocate(int count)
    {
        if (vertexCount != count)
        {
            Release();
            screenVerts = new Vec4f[count];
            viewNormals = new Vec4f[count];
            vertexCount = count;
        }
    }

    void Release()
    {
        delete[] screenVerts;
        delete[] viewNormals;
        screenVerts = nullptr;
        viewNormals = nullptr;
        vertexCount = 0;
    }
};

#endif
