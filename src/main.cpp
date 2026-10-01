#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "RenderObject.h"
#include "SimpleShader.h"
#include "TextOverlay.h"
#include "Platform/Window.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define IMAGE_DIRECTORY "resources/image/"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const std::string TITLE = "REnderer";

// 渲染上下文：颜色/深度缓冲及其描述
struct RenderContext
{
    int width = 0;
    int height = 0;
    PixelFormat format = PixelFormat::ARGB8888;
    float* depth = nullptr;
    std::uint32_t* color = nullptr;
};

int main(int argc, char** argv)
{
    // 自检模式：Renderer --test-mesh <模型路径>，加载模型打印统计信息后退出，不开窗口
    if (argc >= 3 && std::string(argv[1]) == "--test-mesh")
    {
        Mesh testMesh = Mesh::LoadFromFile(argv[2], false);
        if (testMesh.triangleCount == 0)
        {
            std::fprintf(stderr, "自检失败：模型加载为空\n");
            return 1;
        }

        Vec4 lo = testMesh.vertices[0];
        Vec4 hi = testMesh.vertices[0];
        for (int i = 0; i < testMesh.triangleCount * 3; i++)
        {
            const Vec4& p = testMesh.vertices[i];
            lo.x = p.x < lo.x ? p.x : lo.x; lo.y = p.y < lo.y ? p.y : lo.y; lo.z = p.z < lo.z ? p.z : lo.z;
            hi.x = p.x > hi.x ? p.x : hi.x; hi.y = p.y > hi.y ? p.y : hi.y; hi.z = p.z > hi.z ? p.z : hi.z;
        }
        std::printf("三角形数: %d\n", testMesh.triangleCount);
        std::printf("包围盒: min(%f, %f, %f) max(%f, %f, %f)\n", lo.x, lo.y, lo.z, hi.x, hi.y, hi.z);
        std::printf("首顶点: pos(%f, %f, %f) normal(%f, %f, %f) uv(%f, %f)\n",
                    testMesh.vertices[0].x, testMesh.vertices[0].y, testMesh.vertices[0].z,
                    testMesh.normals[0].x, testMesh.normals[0].y, testMesh.normals[0].z,
                    testMesh.uvs[0].x, testMesh.uvs[0].y);
        return 0;
    }

    Window window;
    if (!window.Create(TITLE, SCREEN_WIDTH, SCREEN_HEIGHT))
    {
        return -1;
    }

    // 使用stbi读取图片
    int imgWidth, imgHeight, imgChannels;
    std::string imgPath = IMAGE_DIRECTORY + std::string("test.png");

    // stbi_load 会自动分配内存，使用完后需要用 stbi_image_free 释放
    // 最后一个参数 4 表示强制转换为 RGBA 4通道
    unsigned char* pixelData = stbi_load(
        imgPath.c_str(),
        &imgWidth,
        &imgHeight,
        &imgChannels,
        4  // 强制 RGBA 格式
    );

    if (pixelData == nullptr)
    {
        std::cerr << "Failed to load image: " << imgPath << std::endl;
        std::cerr << "Error: " << stbi_failure_reason() << std::endl;
        return -1;
    }

    Mesh cube = Mesh::CreateCube();
    // 命令行传入模型路径时加载外部模型（.obj/.fbx），失败则回退到内置立方体
    if (argc > 1 && argv[1][0] != '-')
    {
        Mesh loaded = Mesh::LoadFromFile(argv[1]);
        if (loaded.triangleCount > 0)
        {
            cube = std::move(loaded);
        }
        else
        {
            std::cerr << "模型加载失败，回退到内置立方体: " << argv[1] << std::endl;
        }
    }
    Material* material = new Material(new SimpleShader());
    material->SetTexture(pixelData, imgWidth, imgHeight);
    RenderObject renderObject(std::move(cube), material);
    renderObject.Setup();

    int totalPixels = SCREEN_WIDTH * SCREEN_HEIGHT;

    RenderContext renderContext;
    renderContext.width = SCREEN_WIDTH;
    renderContext.height = SCREEN_HEIGHT;
    renderContext.format = Window::Format();
    renderContext.depth = new float[totalPixels];
    renderContext.color = new std::uint32_t[totalPixels];

    Camera camera;
    Light light;

    // FPS 显示（指数滑动平均，避免瞬时抖动）
    float fps = 0.0f;

    // 事件循环
    while (window.PollEvents())
    {
        const InputState& input = window.Input();

        if (input.IsKeyDown(Key::Escape))
        {
            break;
        }

        float dt = window.DeltaTime();
        if (dt > 0.0f)
        {
            float instantFps = 1.0f / dt;
            fps = (fps <= 0.0f) ? instantFps : fps * 0.9f + instantFps * 0.1f;
        }

        camera.ProcessKeyboard(input, dt);

        if (input.IsMouseDown(MouseButton::Right))
        {
            camera.ProcessMouse(input.MouseDeltaX(), input.MouseDeltaY());
        }

        float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
        UniformBuffer ub;
        ub.view = camera.GetViewMatrix();
        ub.projection = camera.GetProjectionMatrix(aspect);
        ub.cameraPos = camera.position;
        ub.lightDir = light.GetDirection();
        ub.lightColor = light.GetColor();
        Shader::SetUniforms(ub);

        renderObject.Render(renderContext.width, renderContext.height,
                            renderContext.format, renderContext.depth,
                            renderContext.color);

        // 在右上角叠加帧数（帧缓冲 Y 向上，右上角即 x 靠右、y 靠 height-1）
        char fpsText[16];
        std::snprintf(fpsText, sizeof(fpsText), "FPS: %d", static_cast<int>(fps + 0.5f));
        int fpsWidth = TextOverlay::Measure(fpsText);
        TextOverlay::Draw(renderContext.color, renderContext.width, renderContext.height,
                          renderContext.format, fpsText,
                          renderContext.width - fpsWidth - 8,
                          renderContext.height - 1 - 8);

        if (!window.Present(renderContext.color, SCREEN_WIDTH, SCREEN_HEIGHT))
        {
            break;
        }
    }

    stbi_image_free(pixelData);
    delete[] renderContext.depth;
    delete[] renderContext.color;
    return 0;
}
