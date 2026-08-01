#ifndef RENDERER_RENDEROBJECT_INCLUDE
#define RENDERER_RENDEROBJECT_INCLUDE

#include "Mesh.h"
#include "Shader.h"
#include "Render/RenderResources.h"

class RenderObject
{
    Mesh mesh;
    Shader* shader;

public:
    RenderObject(Mesh&& m, Shader* s)
        : mesh(std::move(m)), shader(s)
    {
    }

    ~RenderObject()
    {
        delete shader;
    }

    RenderObject(const RenderObject&) = delete;
    RenderObject& operator=(const RenderObject&) = delete;

    RenderObject(RenderObject&& other) noexcept
        : mesh(std::move(other.mesh)), shader(other.shader)
    {
        other.shader = nullptr;
    }

    void Setup()
    {
    }

    void Render(const RenderContext& ctx)
    {
        shader->Render(mesh, ctx);
    }
};

#endif
