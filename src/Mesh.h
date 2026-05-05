#ifndef RENDERER_MESH_INCLUDE
#define RENDERER_MESH_INCLUDE

#include "Math.h"

class Mesh
{
public:
    int triangleCount = 0;
    Vec4* vertices = nullptr;

    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    static Mesh CreateCube();
};

#endif