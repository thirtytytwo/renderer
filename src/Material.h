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

    // ---------------- 反射式查询接口 ----------------
    // C++ 没有真正的反射，这里用「字符串名字 + 虚函数」模拟：
    // 着色器按名字询问材质，不需要在编译期知道子类持有哪些成员变量，
    // 材质上没有对应属性时回退到基类默认实现，不影响编译与运行。

    // 按名字采样贴图。
    // 基类只有一张主贴图，忽略名字直接采样主贴图；
    // 子类可按名字分发到不同贴图，未识别的名字须回退到基类实现。
    virtual Vec4f SampleTexture(const char* name, float u, float v) const
    {
        (void)name;
        return Sample(u, v);
    }

    // 按名字获取标量属性。
    // 基类不持有任何属性，直接返回调用方给的默认值；
    // 子类重写以暴露自身属性（如 roughness / metallic），
    // 未识别的名字须回退到基类实现。
    virtual float GetFloat(const char* name, float defaultValue) const
    {
        (void)name;
        return defaultValue;
    }

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
