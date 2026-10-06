#ifndef RENDERER_PBR_SHADER_INCLUDE
#define RENDERER_PBR_SHADER_INCLUDE

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "Material.h"
#include "Shader.h"

// PBR 着色器：Cook-Torrance 微表面 BRDF。
//   D: GGX/Trowbridge-Reitz 法线分布
//   G: Smith 联合遮蔽（Schlick-GGX 近似）
//   F: Schlick 菲涅尔
// 暂不考虑阴影与 IBL，仅单盏平行光 + 微弱环境项。
// 材质参数（albedo / roughness / metallic）全部经 Material 的反射式接口
// 按名字查询，本类不依赖任何具体材质类型。
class PBRShader : public Shader
{
protected:
    void Vertex(Mesh& mesh, int width, int height) override
    {
        Mat4 model = Mat4::Identity();
        Mat4 mvp = uniforms.projection * uniforms.view * model;

        int vertexCount = mesh.triangleCount * 3;
        pixelBuffer.Allocate(vertexCount);

        for (int i = 0; i < vertexCount; i++)
        {
            Vec4f pos(mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z, 1.0f);

            Vec4f clipPos = mvp * pos;

            if (clipPos.w <= 0.0f)
            {
                pixelBuffer.screenVerts[i] = Vec4f(-1e6f, -1e6f, 0, 0);
                pixelBuffer.viewNormals[i] = Vec4f(0, 0, 0, 0);
                pixelBuffer.uvs[i] = Vec4f(0, 0, 0, 0);
                pixelBuffer.worldPositions[i] = Vec4f(0, 0, 0, 0);
                pixelBuffer.invWs[i] = 0.0f;
                continue;
            }

            Vec4f ndcPos = clipPos / Vec4f(clipPos.w);

            float screenX = (ndcPos.x + 1.0f) * 0.5f * width;
            float screenY = (ndcPos.y + 1.0f) * 0.5f * height;

            pixelBuffer.screenVerts[i] = Vec4f(screenX, screenY, ndcPos.z, 1.0f);
            pixelBuffer.invWs[i] = 1.0f / clipPos.w;

            Vec4f worldNormal = model * mesh.normals[i];
            worldNormal.w = 0.0f;
            pixelBuffer.viewNormals[i] = worldNormal.normalized();

            // 世界空间坐标，像素阶段用来算视线方向
            pixelBuffer.worldPositions[i] = model * pos;

            pixelBuffer.uvs[i] = mesh.uvs[i];
        }
    }

    std::uint32_t Pixel(std::uint32_t& pixel, PixelFormat format, const Vec4f& worldPos,
                        const Vec4f& normal, const Vec4f& uv) override
    {
        (void)pixel;

        // ---- 材质参数：反射式查询，材质上没有该属性时取默认值 ----
        Vec4f albedo = material->SampleTexture("albedo", uv.x, uv.y);
        float roughness = std::clamp(material->GetFloat("roughness", 0.5f), 0.045f, 1.0f);
        float metallic = std::clamp(material->GetFloat("metallic", 0.0f), 0.0f, 1.0f);

        // ---- 几何向量 ----
        // 注意 Vec4f::dot 是含 w 的四分量点积，方向向量的 w 一律清零
        Vec4f n = normal.normalized();
        Vec4f v = uniforms.cameraPos - worldPos;
        v.w = 0.0f;
        v = v.normalized();
        Vec4f l = -uniforms.lightDir;
        l.w = 0.0f;
        l = l.normalized();
        Vec4f h = (v + l).normalized();

        float NdotV = std::max(n.dot(v), 0.0f);
        float NdotL = std::max(n.dot(l), 0.0f);
        float NdotH = std::max(n.dot(h), 0.0f);
        float HdotV = std::max(h.dot(v), 0.0f);

        // ---- F0：非金属约 0.04，金属取 albedo ----
        Vec4f F0(0.04f, 0.04f, 0.04f, 0.0f);
        F0 = F0 * (1.0f - metallic) + albedo * metallic;

        // ---- D：GGX/Trowbridge-Reitz 法线分布 ----
        float a = roughness * roughness;
        float a2 = a * a;
        float dDenom = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
        float D = a2 / (Math::PI * dDenom * dDenom);

        // ---- G：Smith 联合遮蔽，Schlick-GGX 近似（直接光照 k = (r+1)^2 / 8）----
        float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
        float gv = NdotV / (NdotV * (1.0f - k) + k);
        float gl = NdotL / (NdotL * (1.0f - k) + k);
        float G = gv * gl;

        // ---- F：Schlick 菲涅尔 ----
        float fresnelPow = std::pow(1.0f - HdotV, 5.0f);
        Vec4f F = F0 + (Vec4f(1.0f, 1.0f, 1.0f, 0.0f) - F0) * fresnelPow;

        // 镜面反射辐射率，分母加 1e-4 防止掠射角除零
        Vec4f specular = F * (D * G / (4.0f * NdotV * NdotL + 1e-4f));

        // ---- 漫反射：菲涅尔剩余能量，金属无漫反射 ----
        Vec4f kd = (Vec4f(1.0f, 1.0f, 1.0f, 0.0f) - F) * (1.0f - metallic);
        Vec4f diffuse = kd * albedo * (1.0f / Math::PI);

        // ---- 合成：辐射度 × BRDF × NdotL（无阴影项）----
        Vec4f color = (diffuse + specular) * uniforms.lightColor * NdotL;

        // 微弱环境项，避免背光面纯黑
        color += albedo * 0.03f;

        // HDR -> LDR：Reinhard 色调映射 + gamma 校正
        float r = std::pow(color.x / (color.x + 1.0f), 1.0f / 2.2f);
        float g = std::pow(color.y / (color.y + 1.0f), 1.0f / 2.2f);
        float b = std::pow(color.z / (color.z + 1.0f), 1.0f / 2.2f);

        return PackColorF(format, r, g, b, albedo.w);
    }
};

#endif
