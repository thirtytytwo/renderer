#include "Mesh.h"

Mesh::Mesh(Mesh&& other) noexcept
    : triangleCount(other.triangleCount), vertices(other.vertices), normals(other.normals), uvs(other.uvs)
{
    other.triangleCount = 0;
    other.vertices = nullptr;
    other.normals = nullptr;
    other.uvs = nullptr;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        delete[] vertices;
        delete[] normals;
        delete[] uvs;
        triangleCount = other.triangleCount;
        vertices = other.vertices;
        normals = other.normals;
        uvs = other.uvs;
        other.triangleCount = 0;
        other.vertices = nullptr;
        other.normals = nullptr;
        other.uvs = nullptr;
    }
    return *this;
}

Mesh::~Mesh()
{
    delete[] vertices;
    delete[] normals;
    delete[] uvs;
}

Mesh Mesh::CreateSphere(float radius, int segments)
{
    if (segments < 3) segments = 3;

    Mesh mesh;
    int latSegments = segments; // 纬度分段（北极 → 南极）
    int lonSegments = segments; // 经度分段（绕 y 轴一周）

    // 极点处每格有一个面积为 0 的退化三角形，光栅化时被 area > 0 测试自然剔除
    mesh.triangleCount = latSegments * lonSegments * 2;
    int vertexCount = mesh.triangleCount * 3;
    mesh.vertices = new Vec4[vertexCount];
    mesh.normals = new Vec4[vertexCount];
    mesh.uvs = new Vec4[vertexCount];

    // 球面参数化：theta 纬度角（北极 0 → 南极 π），phi 经度角（0 → 2π，绕 y 轴）
    auto point = [&](int lat, int lon) -> Vec4
    {
        float theta = Math::PI * (float)lat / (float)latSegments;
        float phi = 2.0f * Math::PI * (float)lon / (float)lonSegments;
        float sinTheta = std::sin(theta);
        return Vec4(radius * sinTheta * std::cos(phi),
                    radius * std::cos(theta),
                    radius * sinTheta * std::sin(phi),
                    0.0f);
    };
    // v 由北极到南极 0 → 1，符合「v 向下，(0,0) 为图像顶部」的采样约定
    auto uv = [&](int lat, int lon) -> Vec4
    {
        return Vec4((float)lon / (float)lonSegments, (float)lat / (float)latSegments, 0.0f, 0.0f);
    };

    int idx = 0;
    for (int i = 0; i < latSegments; i++)
    {
        for (int j = 0; j < lonSegments; j++)
        {
            Vec4 p00 = point(i, j);
            Vec4 p01 = point(i, j + 1);
            Vec4 p10 = point(i + 1, j);
            Vec4 p11 = point(i + 1, j + 1);

            // 绕序保证从球外侧看为逆时针（配合光栅化 area > 0 的正面判定）
            Vec4 tri[2][3] = {
                { p00, p11, p10 },
                { p00, p01, p11 }
            };
            Vec4 triUV[2][3] = {
                { uv(i, j), uv(i + 1, j + 1), uv(i + 1, j) },
                { uv(i, j), uv(i, j + 1),     uv(i + 1, j + 1) }
            };

            for (int t = 0; t < 2; t++)
            {
                for (int v = 0; v < 3; v++)
                {
                    mesh.vertices[idx] = tri[t][v];
                    // 球面法线即归一化位置（w 已为 0，归一化不受 w 影响）
                    mesh.normals[idx] = tri[t][v].normalized();
                    mesh.uvs[idx] = triUV[t][v];
                    idx++;
                }
            }
        }
    }

    return mesh;
}

Mesh Mesh::CreateCube()
{
    Mesh mesh;
    mesh.triangleCount = 12;
    mesh.vertices = new Vec4[36];
    mesh.normals = new Vec4[36];

    Vec4 v0(-0.5f, -0.5f, -0.5f, 0.0f);
    Vec4 v1( 0.5f, -0.5f, -0.5f, 0.0f);
    Vec4 v2( 0.5f,  0.5f, -0.5f, 0.0f);
    Vec4 v3(-0.5f,  0.5f, -0.5f, 0.0f);
    Vec4 v4(-0.5f, -0.5f,  0.5f, 0.0f);
    Vec4 v5( 0.5f, -0.5f,  0.5f, 0.0f);
    Vec4 v6( 0.5f,  0.5f,  0.5f, 0.0f);
    Vec4 v7(-0.5f,  0.5f,  0.5f, 0.0f);

    Vec4 nFront(0.0f, 0.0f, 1.0f, 0.0f);
    Vec4 nBack(0.0f, 0.0f, -1.0f, 0.0f);
    Vec4 nLeft(-1.0f, 0.0f, 0.0f, 0.0f);
    Vec4 nRight(1.0f, 0.0f, 0.0f, 0.0f);
    Vec4 nUp(0.0f, 1.0f, 0.0f, 0.0f);
    Vec4 nDown(0.0f, -1.0f, 0.0f, 0.0f);

    // 前面 (z = 0.5)
    mesh.vertices[0]  = v4; mesh.vertices[1]  = v5; mesh.vertices[2]  = v6;
    mesh.vertices[3]  = v4; mesh.vertices[4]  = v6; mesh.vertices[5]  = v7;
    for (int i = 0; i < 6; i++) mesh.normals[i] = nFront;

    // 后面 (z = -0.5)
    mesh.vertices[6]  = v1; mesh.vertices[7]  = v0; mesh.vertices[8]  = v3;
    mesh.vertices[9]  = v1; mesh.vertices[10] = v3; mesh.vertices[11] = v2;
    for (int i = 6; i < 12; i++) mesh.normals[i] = nBack;

    // 左面 (x = -0.5)
    mesh.vertices[12] = v0; mesh.vertices[13] = v4; mesh.vertices[14] = v7;
    mesh.vertices[15] = v0; mesh.vertices[16] = v7; mesh.vertices[17] = v3;
    for (int i = 12; i < 18; i++) mesh.normals[i] = nLeft;

    // 右面 (x = 0.5)
    mesh.vertices[18] = v5; mesh.vertices[19] = v1; mesh.vertices[20] = v2;
    mesh.vertices[21] = v5; mesh.vertices[22] = v2; mesh.vertices[23] = v6;
    for (int i = 18; i < 24; i++) mesh.normals[i] = nRight;

    // 上面 (y = 0.5)
    mesh.vertices[24] = v7; mesh.vertices[25] = v6; mesh.vertices[26] = v2;
    mesh.vertices[27] = v7; mesh.vertices[28] = v2; mesh.vertices[29] = v3;
    for (int i = 24; i < 30; i++) mesh.normals[i] = nUp;

    // 下面 (y = -0.5)
    mesh.vertices[30] = v0; mesh.vertices[31] = v1; mesh.vertices[32] = v5;
    mesh.vertices[33] = v0; mesh.vertices[34] = v5; mesh.vertices[35] = v4;
    for (int i = 30; i < 36; i++) mesh.normals[i] = nDown;

    // UV 展开：六个面的顶点绕序均对应同一模式，
    // 从各面外侧看贴图保持正立（u 向右，v 向下，(0,0) 为图像顶部）
    mesh.uvs = new Vec4[36];
    const Vec4 faceUV[6] = {
        Vec4(0.0f, 1.0f, 0.0f, 0.0f), Vec4(1.0f, 1.0f, 0.0f, 0.0f), Vec4(1.0f, 0.0f, 0.0f, 0.0f),
        Vec4(0.0f, 1.0f, 0.0f, 0.0f), Vec4(1.0f, 0.0f, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f)
    };
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < 6; i++)
        {
            mesh.uvs[face * 6 + i] = faceUV[i];
        }
    }

    return mesh;
}