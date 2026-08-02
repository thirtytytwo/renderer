#ifndef RENDERER_RENDERRESOURCES_INCLUDE
#define RENDERER_RENDERRESOURCES_INCLUDE

#include <cstdint>

#include "../Platform/PixelFormat.h"

struct RenderContext
{
    int width = 0;
    int height = 0;
    PixelFormat format = PixelFormat::ARGB8888;
    float* depth = nullptr;
    std::uint32_t* color = nullptr;
};

#endif
