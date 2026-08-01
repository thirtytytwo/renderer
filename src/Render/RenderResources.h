#ifndef RENDERER_RENDERRESOURCES_INCLUDE
#define RENDERER_RENDERRESOURCES_INCLUDE

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_pixels.h>

struct RenderContext
{
    int width;
    int height;
    SDL_PixelFormat format;
    float* depth = nullptr;
    Uint32* color = nullptr;
};

#endif
