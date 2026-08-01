#include <iostream>
#include <string>
#include <cstring>

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "Camera.h"
#include "Light.h"
#include "RenderObject.h"
#include "SimpleShader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define IMAGE_DIRECTORY "resources/image/"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const std::string TITLE = "REnderer";

int main(int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        std::cerr << "SDL cant initialize!" << std::endl;
        return -1;
    }

    SDL_Window* WindowHandle = SDL_CreateWindow(
        TITLE.c_str(),
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );

    if(WindowHandle == nullptr)
    {
        std::cerr << "Window create failed!" << std::endl;
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
        SDL_DestroyWindow(WindowHandle);
        SDL_Quit();
        return -1;
    }
    

    SDL_Surface* SurfaceHandle = SDL_GetWindowSurface(WindowHandle);

    if(SurfaceHandle == nullptr)
    {
        std::cerr << "Surface create failed!" << std::endl;
        stbi_image_free(pixelData);
        SDL_DestroyWindow(WindowHandle);
        SDL_Quit();
        return -1;
    }

    Mesh cube = Mesh::CreateCube();
    RenderObject renderObject(std::move(cube), new SimpleShader());
    renderObject.Setup();

    RenderContext renderContext;
    renderContext.width = SCREEN_WIDTH;
    renderContext.height = SCREEN_HEIGHT;
    renderContext.format = SurfaceHandle->format;
    int totalPixels = SCREEN_WIDTH * SCREEN_HEIGHT;
    renderContext.depth = new float[totalPixels];
    renderContext.color = new Uint32[totalPixels];

    Camera camera;
    Light light;

    bool rightMouseDown = false;
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;

    Uint64 lastTime = SDL_GetTicks();

    // 事件循环
    bool isRunning = true;
    SDL_Event event;

    while (isRunning)
    {
        // 处理所有待处理的事件
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    isRunning = false;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE)
                    {
                        isRunning = false;
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_RIGHT)
                    {
                        rightMouseDown = true;
                        lastMouseX = event.button.x;
                        lastMouseY = event.button.y;
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (event.button.button == SDL_BUTTON_RIGHT)
                    {
                        rightMouseDown = false;
                    }
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    if (rightMouseDown)
                    {
                        float dx = event.motion.x - lastMouseX;
                        float dy = event.motion.y - lastMouseY;
                        lastMouseX = event.motion.x;
                        lastMouseY = event.motion.y;
                        camera.ProcessMouse(dx, dy);
                    }
                    break;

                default:
                    break;
            }
        }

        Uint64 currentTime = SDL_GetTicks();
        float dt = (float)(currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        const bool* keyState = SDL_GetKeyboardState(NULL);
        camera.ProcessKeyboard(keyState, dt);

        float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
        UniformBuffer ub;
        ub.view = camera.GetViewMatrix();
        ub.projection = camera.GetProjectionMatrix(aspect);
        ub.cameraPos = camera.position;
        ub.lightDir = light.GetDirection();
        ub.lightColor = light.GetColor();
        Shader::SetUniforms(ub);

        SDL_LockSurface(SurfaceHandle);
        {
            renderObject.Render(renderContext);
            std::memcpy(SurfaceHandle->pixels, renderContext.color, totalPixels * sizeof(Uint32));
        }
        SDL_UnlockSurface(SurfaceHandle);
        SDL_FlipSurface(SurfaceHandle, SDL_FLIP_VERTICAL);
        SDL_UpdateWindowSurface(WindowHandle);
    }

    stbi_image_free(pixelData);
    delete[] renderContext.depth;
    delete[] renderContext.color;
    SDL_DestroyWindow(WindowHandle);
    SDL_Quit();
    return 0;
}