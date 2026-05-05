#include <iostream>
#include <string>

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

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

    Mesh mesh = Mesh::CreateCube();

    Shader* shader = new SimpleShader();

    // 事件循环
    bool isRunning = true;
    SDL_Event event;

    Mesh cube = Mesh::CreateCube();

    while (isRunning)
    {
        // 处理所有待处理的事件
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    // 窗口关闭按钮点击
                    isRunning = false;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    // 键盘按下事件
                    if (event.key.key == SDLK_ESCAPE)
                    {
                        // 按下 ESC 退出
                        isRunning = false;
                    }
                    break;

                default:
                    break;
            }
        }

        // TODO: 在这里进行渲染逻辑
        SDL_LockSurface(SurfaceHandle);
        {
            SDL_FillSurfaceRect(SurfaceHandle, NULL, 0);
            Uint32* DestPixels = (Uint32*)SurfaceHandle->pixels;
            
            shader->Render(DestPixels, mesh, SCREEN_WIDTH, SCREEN_HEIGHT, SurfaceHandle->format);

            // 使用 OpenMP 并行化
            // #pragma omp parallel for
            // for (int y = 0; y < SCREEN_HEIGHT; y++)
            // {
            //     for (int x = 0; x < SCREEN_WIDTH; x++)
            //     {
            //         DestPixels[y * SCREEN_WIDTH + x] = 0xFF0000FF; // 红色
            //     }
            // }

            // for(int y = 0 ; y < SCREEN_HEIGHT; y++)
            // {
            //     for(int x = 0; x < SCREEN_WIDTH; x++)
            //     {
            //         Uint32 Color = SDL_MapRGB(
            //             SDL_GetPixelFormatDetails(SurfaceHandle->format),
            //             0,
            //             static_cast<uint8_t>(255),
            //             static_cast<uint8_t>(255),
            //             static_cast<uint8_t>()
            //         );
            //         DestPixels[y * SCREEN_WIDTH + x] = Color;
            //     }
            // }
        }
        SDL_UnlockSurface(SurfaceHandle);
        SDL_FlipSurface(SurfaceHandle, SDL_FLIP_VERTICAL);
        // 刷新窗口表面
        SDL_UpdateWindowSurface(WindowHandle);
    }

    stbi_image_free(pixelData);
    SDL_DestroyWindow(WindowHandle);
    SDL_Quit();
    return 0;
}