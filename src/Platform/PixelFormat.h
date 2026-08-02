#ifndef RENDERER_PLATFORM_PIXELFORMAT_INCLUDE
#define RENDERER_PLATFORM_PIXELFORMAT_INCLUDE

#include <cstdint>

// 帧缓冲的像素打包方式。
// 名称按“从最高位到最低位”的通道顺序书写，与具体字节序无关。
enum class PixelFormat : std::uint32_t
{
    ARGB8888,   // A<<24 | R<<16 | G<<8 | B —— 桌面平台（Windows / macOS）原生格式
    ABGR8888,   // A<<24 | B<<16 | G<<8 | R
};

// 将 8 位分量打包为一个 32 位像素
inline std::uint32_t PackColor(PixelFormat format,
                               std::uint8_t r,
                               std::uint8_t g,
                               std::uint8_t b,
                               std::uint8_t a = 255)
{
    if (format == PixelFormat::ABGR8888)
    {
        return (static_cast<std::uint32_t>(a) << 24)
             | (static_cast<std::uint32_t>(b) << 16)
             | (static_cast<std::uint32_t>(g) << 8)
             |  static_cast<std::uint32_t>(r);
    }

    return (static_cast<std::uint32_t>(a) << 24)
         | (static_cast<std::uint32_t>(r) << 16)
         | (static_cast<std::uint32_t>(g) << 8)
         |  static_cast<std::uint32_t>(b);
}

// 浮点颜色（0~1）打包，自动做钳制
inline std::uint32_t PackColorF(PixelFormat format, float r, float g, float b, float a = 1.0f)
{
    auto toByte = [](float v) -> std::uint8_t
    {
        if (v <= 0.0f) return 0;
        if (v >= 1.0f) return 255;
        return static_cast<std::uint8_t>(v * 255.0f + 0.5f);
    };

    return PackColor(format, toByte(r), toByte(g), toByte(b), toByte(a));
}

#endif
