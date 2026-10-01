#ifndef RENDERER_RENDEROBJECT_INCLUDE
#define RENDERER_RENDEROBJECT_INCLUDE

#include <cstdint>
#include <utility>

#include "Mesh.h"
#include "Material.h"
#include "Platform/PixelFormat.h"

class RenderObject
{
    Mesh mesh;
    Material* material;

public:
    RenderObject(Mesh&& m, Material* mat)
        : mesh(std::move(m)), material(mat)
    {
    }

    ~RenderObject()
    {
        delete material;
    }

    RenderObject(const RenderObject&) = delete;
    RenderObject& operator=(const RenderObject&) = delete;

    RenderObject(RenderObject&& other) noexcept
        : mesh(std::move(other.mesh)), material(other.material)
    {
        other.material = nullptr;
    }

    void Setup()
    {
    }

    void Render(int width, int height, PixelFormat format,
                float* depthBuffer, std::uint32_t* colorBuffer)
    {
        material->GetShader()->Render(mesh, width, height, format,
                                      depthBuffer, colorBuffer);
    }
};

#endif
