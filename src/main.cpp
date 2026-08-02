#include <cstdint>
#include <iostream>
#include <string>

#include "Camera.h"
#include "Light.h"
#include "RenderObject.h"
#include "SimpleShader.h"
#include "Platform/Window.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define IMAGE_DIRECTORY "resources/image/"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const std::string TITLE = "REnderer";

int main(int argc, char** argv)
{
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
    RenderObject renderObject(std::move(cube), new SimpleShader());
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

    // 事件循环
    while (window.PollEvents())
    {
        const InputState& input = window.Input();

        if (input.IsKeyDown(Key::Escape))
        {
            break;
        }

        float dt = window.DeltaTime();

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

        renderObject.Render(renderContext);

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
