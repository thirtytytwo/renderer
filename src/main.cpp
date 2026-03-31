#include <iostream>
#include <string>

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define IMAGE_DIRECTORY "resources/image/"

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

int main(int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        return -1;
    }

    int imgWidth, imgHeight, imgChannels;
    std::string imgPath = IMAGE_DIRECTORY + std::string("test.png");

    unsigned char* pixels = stbi_load(imgPath.c_str(), &imgWidth, &imgHeight, &imgChannels, STBI_rgb_alpha);
    if(!pixels)
    {
        std::cout << "can load picture" << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL Window",
        SCREEN_WIDTH, SCREEN_HEIGHT,
        0
    );

    if (!window)
    {
        std::cout << "cant get window" << std::endl;
        stbi_image_free(pixels);
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        stbi_image_free(pixels);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    int width = imgWidth;
    int height = imgHeight;
    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        width, height,
        SDL_PIXELFORMAT_RGBA32,
        pixels, width*4
    );
    
    if (!surface)
    {
        std::cout << "cant init surface" << std::endl;
        stbi_image_free(pixels);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    SDL_DestroySurface(surface);
    stbi_image_free(pixels);

    if (!texture)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    bool running = true;
    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_ESCAPE)
                {
                    running = false;
                }
            }
        }

        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}