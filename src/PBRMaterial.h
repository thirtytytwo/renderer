#ifndef RENDERER_PBR_MATERIAL_INCLUDE
#define RENDERER_PBR_MATERIAL_INCLUDE

#include <algorithm>
#include <cstring>

#include "Material.h"

// PBR 材质球：在基类主贴图（albedo）之外持有 PBR 参数。
// 所有参数通过基类的反射式接口按名字暴露，
// 着色器无需 include 本头文件即可读取，编译期不与成员变量强相关。
class PBRMaterial : public Material
{
    float roughness = 0.5f; // 粗糙度 [0,1]，越小高光越锐利
    float metallic = 0.0f;  // 金属度 [0,1]，1 为纯金属

public:
    explicit PBRMaterial(Shader* s)
        : Material(s)
    {
    }

    // 下限 0.045：避免粗糙度为 0 时 GGX 法线分布出现除零
    void SetRoughness(float r) { roughness = std::clamp(r, 0.045f, 1.0f); }
    void SetMetallic(float m) { metallic = std::clamp(m, 0.0f, 1.0f); }

    float GetFloat(const char* name, float defaultValue) const override
    {
        if (std::strcmp(name, "roughness") == 0) return roughness;
        if (std::strcmp(name, "metallic") == 0) return metallic;
        // 未识别的名字回退基类（返回默认值），不报错
        return Material::GetFloat(name, defaultValue);
    }
};

#endif
