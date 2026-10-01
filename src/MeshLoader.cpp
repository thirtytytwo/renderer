// 网格文件加载：把第三方库解析出的索引网格展平成 Mesh 的三角形汤顶点流。
// OBJ 走 tinyobjloader，FBX 走 ufbx，均不加载材质。
#include "Mesh.h"

#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

#include <ufbx.h>
#include <tiny_obj_loader.h>

namespace
{
// 单个角点：位置 + 可选法线 / UV
struct Corner
{
    Vec4 pos;
    Vec4 normal;
    Vec4 uv;
    bool hasNormal = false;
};

// 三角形汤收集器
struct TriangleSoup
{
    std::vector<Vec4> positions;
    std::vector<Vec4> normals;
    std::vector<Vec4> uvs;

    void Reserve(size_t triangleCount)
    {
        positions.reserve(triangleCount * 3);
        normals.reserve(triangleCount * 3);
        uvs.reserve(triangleCount * 3);
    }

    // 追加一个三角形；任一顶点缺法线时整个三角形退化为面法线，
    // 保证 Shader 访问 normals[] 时始终有有效值
    void PushTriangle(const Corner corners[3])
    {
        Vec4 faceNormal(0.0f, 0.0f, 0.0f, 0.0f);
        bool needFaceNormal = !(corners[0].hasNormal && corners[1].hasNormal && corners[2].hasNormal);
        if (needFaceNormal)
        {
            // 右手系 CCW 绕序，叉积即外法线（与渲染器 area > 0 剔除约定一致）
            Vec4 e1 = corners[1].pos - corners[0].pos;
            Vec4 e2 = corners[2].pos - corners[0].pos;
            faceNormal = e1.cross(e2).normalized();
        }

        for (int i = 0; i < 3; i++)
        {
            positions.push_back(corners[i].pos);
            normals.push_back(corners[i].hasNormal ? corners[i].normal : faceNormal);
            uvs.push_back(corners[i].uv);
        }
    }
};

std::string LowerExtension(const char* path)
{
    std::string p(path);
    size_t dot = p.find_last_of('.');
    if (dot == std::string::npos) return "";
    std::string ext = p.substr(dot + 1);
    for (char& c : ext) c = (char)std::tolower((unsigned char)c);
    return ext;
}

bool LoadOBJ(const char* path, TriangleSoup& soup)
{
    tinyobj::ObjReaderConfig config;
    config.mtl_search_path = "";  // 不加载材质
    config.triangulate = true;    // 多边形面自动三角化

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path, config))
    {
        std::fprintf(stderr, "[Mesh] OBJ 加载失败: %s\n%s", path, reader.Error().c_str());
        return false;
    }
    if (!reader.Warning().empty())
    {
        std::fprintf(stderr, "[Mesh] OBJ 警告: %s", reader.Warning().c_str());
    }

    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();

    size_t triangleCount = 0;
    for (const tinyobj::shape_t& shape : shapes)
        triangleCount += shape.mesh.num_face_vertices.size();
    soup.Reserve(triangleCount);

    for (const tinyobj::shape_t& shape : shapes)
    {
        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
        {
            size_t fv = shape.mesh.num_face_vertices[f];
            // 扇形展开，防御未三角化的多边形面
            for (size_t k = 0; k + 2 < fv; k++)
            {
                const size_t cornerIndex[3] = { 0, k + 1, k + 2 };
                Corner corners[3];
                for (int c = 0; c < 3; c++)
                {
                    const tinyobj::index_t idx = shape.mesh.indices[indexOffset + cornerIndex[c]];

                    corners[c].pos = Vec4(
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2],
                        0.0f);

                    corners[c].hasNormal = idx.normal_index >= 0;
                    if (corners[c].hasNormal)
                    {
                        corners[c].normal = Vec4(
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2],
                            0.0f);
                    }

                    if (idx.texcoord_index >= 0)
                    {
                        corners[c].uv = Vec4(
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            attrib.texcoords[2 * idx.texcoord_index + 1],
                            0.0f, 0.0f);
                    }
                }
                soup.PushTriangle(corners);
            }
            indexOffset += fv;
        }
    }
    return true;
}

bool LoadFBX(const char* path, TriangleSoup& soup)
{
    ufbx_load_opts opts = {};
    // 统一坐标系 / 单位：右手系 Y 向上（与渲染器约定一致），长度单位换算为米
    opts.target_axes = ufbx_axes_right_handed_y_up;
    opts.target_unit_meters = 1.0f;
    // 几何变换直接烘焙进顶点，展平节点层级
    opts.geometry_transform_handling = UFBX_GEOMETRY_TRANSFORM_HANDLING_MODIFY_GEOMETRY;
    opts.generate_missing_normals = true;

    ufbx_error error;
    ufbx_scene* scene = ufbx_load_file(path, &opts, &error);
    if (scene == nullptr)
    {
        std::fprintf(stderr, "[Mesh] FBX 加载失败: %s\n%.*s\n",
                     path, (int)error.description.length, error.description.data);
        return false;
    }

    std::vector<uint32_t> triIndices;
    for (const ufbx_node* node : scene->nodes)
    {
        const ufbx_mesh* mesh = node->mesh;
        if (mesh == nullptr) continue;

        // 法线矩阵 = geometry_to_world 的逆转置
        const ufbx_matrix normalMatrix = ufbx_matrix_for_normals(&node->geometry_to_world);

        for (ufbx_face face : mesh->faces)
        {
            triIndices.resize(face.num_indices * 3);
            // 返回值是三角形个数，索引缓冲中为每个三角形 3 个角点索引
            uint32_t numTriangles = ufbx_triangulate_face(triIndices.data(), triIndices.size(), mesh, face);

            for (uint32_t tri = 0; tri < numTriangles; tri++)
            {
                Corner corners[3];
                for (int c = 0; c < 3; c++)
                {
                    uint32_t index = triIndices[tri * 3 + c];

                    ufbx_vec3 p = ufbx_get_vertex_vec3(&mesh->vertex_position, index);
                    p = ufbx_transform_position(&node->geometry_to_world, p);
                    corners[c].pos = Vec4((float)p.x, (float)p.y, (float)p.z, 0.0f);

                    corners[c].hasNormal = mesh->vertex_normal.exists;
                    if (corners[c].hasNormal)
                    {
                        ufbx_vec3 n = ufbx_get_vertex_vec3(&mesh->vertex_normal, index);
                        n = ufbx_transform_direction(&normalMatrix, n);
                        corners[c].normal = Vec4((float)n.x, (float)n.y, (float)n.z, 0.0f);
                    }

                    if (mesh->vertex_uv.exists)
                    {
                        ufbx_vec2 uv = ufbx_get_vertex_vec2(&mesh->vertex_uv, index);
                        corners[c].uv = Vec4((float)uv.x, (float)uv.y, 0.0f, 0.0f);
                    }
                }
                soup.PushTriangle(corners);
            }
        }
    }

    ufbx_free_scene(scene);
    return true;
}

// 平移到原点居中，统一缩放至最长边为 1
void NormalizeModel(TriangleSoup& soup)
{
    if (soup.positions.empty()) return;

    Vec4 lo = soup.positions[0];
    Vec4 hi = soup.positions[0];
    for (const Vec4& p : soup.positions)
    {
        lo.x = p.x < lo.x ? p.x : lo.x;
        lo.y = p.y < lo.y ? p.y : lo.y;
        lo.z = p.z < lo.z ? p.z : lo.z;
        hi.x = p.x > hi.x ? p.x : hi.x;
        hi.y = p.y > hi.y ? p.y : hi.y;
        hi.z = p.z > hi.z ? p.z : hi.z;
    }

    float extent = hi.x - lo.x;
    extent = (hi.y - lo.y) > extent ? (hi.y - lo.y) : extent;
    extent = (hi.z - lo.z) > extent ? (hi.z - lo.z) : extent;
    if (extent <= 0.0f) return;

    Vec4 center = (lo + hi) * 0.5f;
    float scale = 1.0f / extent;
    for (Vec4& p : soup.positions)
    {
        p = (p - center) * scale;
        p.w = 0.0f;
    }
}
} // namespace

Mesh Mesh::LoadFromFile(const char* path, bool normalize)
{
    Mesh mesh;
    TriangleSoup soup;

    const std::string ext = LowerExtension(path);
    bool ok = false;
    if (ext == "obj")
    {
        ok = LoadOBJ(path, soup);
    }
    else if (ext == "fbx")
    {
        ok = LoadFBX(path, soup);
    }
    else
    {
        std::fprintf(stderr, "[Mesh] 不支持的格式: %s（仅支持 .obj / .fbx）\n", path);
        return mesh;
    }

    if (!ok || soup.positions.empty())
    {
        std::fprintf(stderr, "[Mesh] 未能从 %s 解析出任何三角形\n", path);
        return mesh;
    }

    if (normalize)
    {
        NormalizeModel(soup);
    }

    const size_t vertexCount = soup.positions.size();
    mesh.triangleCount = (int)(vertexCount / 3);
    mesh.vertices = new Vec4[vertexCount];
    mesh.normals = new Vec4[vertexCount];
    mesh.uvs = new Vec4[vertexCount];
    for (size_t i = 0; i < vertexCount; i++)
    {
        mesh.vertices[i] = soup.positions[i];
        mesh.normals[i] = soup.normals[i].normalized();
        mesh.uvs[i] = soup.uvs[i];
    }

    std::fprintf(stderr, "[Mesh] 已加载 %s：%d 个三角形\n", path, mesh.triangleCount);
    return mesh;
}
