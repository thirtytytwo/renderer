#ifndef RENDERER_MATERIAL_INCLUDE
#define RENDERER_MATERIAL_INCLUDE

#include <cmath>

#include "Math.h"
#include "Shader.h"

// 材质：持有贴图数据与着色器指针。
// 负责规定贴图的采样位置（UV 约定）与采样逻辑（环绕方式 + 过滤方式）。
// 贴图内存不归 Material 所有，由调用方负责加载与释放。
class Material
{
    Shader* shader;

    // 主色贴图数据，强制 RGBA 8bit 四通道（与 stbi_load 的加载约定一致）
    const unsigned char* texture = nullptr;
    int texWidth = 0;
    int texHeight = 0;

public:
    explicit Material(Shader* s)
        : shader(s)
    {
        // 建立 Shader -> Material 的反向引用，供着色阶段采样贴图
        shader->SetMaterial(this);
    }

    ~Material()
    {
        delete shader;
    }

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    Material(Material&& other) noexcept
        : shader(other.shader)
        , texture(other.texture)
        , texWidth(other.texWidth)
        , texHeight(other.texHeight)
    {
        // 移动后反向指针需要指向新对象
        if (shader) shader->SetMaterial(this);
        other.shader = nullptr;
        other.texture = nullptr;
    }

    void SetTexture(const unsigned char* data, int width, int height)
    {
        texture = data;
        texWidth = width;
        texHeight = height;
    }

    bool HasTexture() const { return texture != nullptr; }

    Shader* GetShader() const { return shader; }

    // 贴图采样。
    // 采样位置约定：UV ∈ [0,1]；u 向右，v 向下，(0,0) 对应贴图第一行（图像顶部）。
    // 采样逻辑：repeat 环绕 + 最近邻过滤，返回 RGBA (0~1) 浮点颜色。
    Vec4f Sample(float u, float v) const
    {
        // 无贴图时返回白色，着色结果退化为无贴图案
        if (!HasTexture() || texWidth <= 0 || texHeight <= 0)
        {
            return Vec4f(1.0f);
        }

        // repeat 环绕：UV 超出 [0,1] 的部分取小数位
        u = u - std::floor(u);
        v = v - std::floor(v);

        // 最近邻：映射到像素中心后取整
        int x = (int)(u * (texWidth - 1) + 0.5f);
        int y = (int)(v * (texHeight - 1) + 0.5f);

        const unsigned char* p = texture + (y * texWidth + x) * 4;
        const float inv255 = 1.0f / 255.0f;
        return Vec4f(p[0] * inv255, p[1] * inv255, p[2] * inv255, p[3] * inv255);
    }
};

#endif
