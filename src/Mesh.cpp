#include "Mesh.h"

Mesh::Mesh(Mesh&& other) noexcept
    : triangleCount(other.triangleCount), vertices(other.vertices)
{
    other.triangleCount = 0;
    other.vertices = nullptr;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        delete[] vertices;
        triangleCount = other.triangleCount;
        vertices = other.vertices;
        other.triangleCount = 0;
        other.vertices = nullptr;
    }
    return *this;
}

Mesh::~Mesh()
{
    delete[] vertices;
}

Mesh Mesh::CreateCube()
{
    Mesh mesh;
    mesh.triangleCount = 12;
    mesh.vertices = new Vec4[36];

    // 正方体8个顶点 (中心在原点, 边长1)
    // 0: (-0.5, -0.5, -0.5)
    // 1: ( 0.5, -0.5, -0.5)
    // 2: ( 0.5,  0.5, -0.5)
    // 3: (-0.5,  0.5, -0.5)
    // 4: (-0.5, -0.5,  0.5)
    // 5: ( 0.5, -0.5,  0.5)
    // 6: ( 0.5,  0.5,  0.5)
    // 7: (-0.5,  0.5,  0.5)

    Vec4 v0(-0.5f, -0.5f, -0.5f, 0.0f);
    Vec4 v1( 0.5f, -0.5f, -0.5f, 0.0f);
    Vec4 v2( 0.5f,  0.5f, -0.5f, 0.0f);
    Vec4 v3(-0.5f,  0.5f, -0.5f, 0.0f);
    Vec4 v4(-0.5f, -0.5f,  0.5f, 0.0f);
    Vec4 v5( 0.5f, -0.5f,  0.5f, 0.0f);
    Vec4 v6( 0.5f,  0.5f,  0.5f, 0.0f);
    Vec4 v7(-0.5f,  0.5f,  0.5f, 0.0f);

    // 前面 (z = 0.5)
    mesh.vertices[0]  = v4; mesh.vertices[1]  = v5; mesh.vertices[2]  = v6;
    mesh.vertices[3]  = v4; mesh.vertices[4]  = v6; mesh.vertices[5]  = v7;
    // 后面 (z = -0.5)
    mesh.vertices[6]  = v1; mesh.vertices[7]  = v0; mesh.vertices[8]  = v3;
    mesh.vertices[9]  = v1; mesh.vertices[10] = v3; mesh.vertices[11] = v2;
    // 左面 (x = -0.5)
    mesh.vertices[12] = v0; mesh.vertices[13] = v4; mesh.vertices[14] = v7;
    mesh.vertices[15] = v0; mesh.vertices[16] = v7; mesh.vertices[17] = v3;
    // 右面 (x = 0.5)
    mesh.vertices[18] = v5; mesh.vertices[19] = v1; mesh.vertices[20] = v2;
    mesh.vertices[21] = v5; mesh.vertices[22] = v2; mesh.vertices[23] = v6;
    // 上面 (y = 0.5)
    mesh.vertices[24] = v7; mesh.vertices[25] = v6; mesh.vertices[26] = v2;
    mesh.vertices[27] = v7; mesh.vertices[28] = v2; mesh.vertices[29] = v3;
    // 下面 (y = -0.5)
    mesh.vertices[30] = v0; mesh.vertices[31] = v1; mesh.vertices[32] = v5;
    mesh.vertices[33] = v0; mesh.vertices[34] = v5; mesh.vertices[35] = v4;

    return mesh;
}