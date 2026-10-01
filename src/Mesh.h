#ifndef RENDERER_MESH_INCLUDE
#define RENDERER_MESH_INCLUDE

#include "Math.h"

class Mesh
{
public:
    int triangleCount = 0;
    Vec4* vertices = nullptr;
    Vec4* normals = nullptr;
    Vec4* uvs = nullptr;    // 顶点 UV，xy 分量有效

    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    static Mesh CreateCube();

    // 从文件加载网格（支持 .obj / .fbx，仅几何数据，不加载材质）。
    // normalize = true 时将模型平移到原点居中、统一缩放至最长边为 1（与 CreateCube 同尺度）。
    // 失败返回空网格（triangleCount == 0），错误信息输出到 stderr。
    static Mesh LoadFromFile(const char* path, bool normalize = true);
};

#endif