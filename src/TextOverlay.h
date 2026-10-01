#ifndef RENDERER_TEXTOVERLAY_INCLUDE
#define RENDERER_TEXTOVERLAY_INCLUDE

#include <cstdint>

#include "Platform/PixelFormat.h"

// 极简位图文字叠加层（5x7 像素字体，2 倍缩放）。
// 直接写入帧缓冲，用于在画面上叠加 FPS 等调试信息。
// 帧缓冲约定：Y 向上（第 0 行为画面底部）。
class TextOverlay
{
public:
    static constexpr int Scale = 2;
    static constexpr int GlyphWidth = 5;
    static constexpr int GlyphHeight = 7;
    static constexpr int CharAdvance = (GlyphWidth + 1) * Scale;  // 含 1 列字符间隔

    // 文本占用的像素宽度（末尾不含字符间隔）
    static int Measure(const char* text)
    {
        int n = 0;
        for (const char* p = text; *p != '\0'; ++p) ++n;
        if (n == 0) return 0;
        return n * CharAdvance - Scale;
    }

    // 把文字画进帧缓冲。
    // (x, yTop) 为文字左上角在缓冲中的坐标；yTop 是字形最高一行所在的缓冲行，
    // 字形向缓冲行号减小的方向（即屏幕下方）展开。
    static void Draw(std::uint32_t* buffer, int bufWidth, int bufHeight,
                     PixelFormat format, const char* text,
                     int x, int yTop,
                     std::uint8_t r = 255, std::uint8_t g = 255, std::uint8_t b = 255)
    {
        if (buffer == nullptr || text == nullptr) return;

        const std::uint32_t pixel = PackColor(format, r, g, b);
        int cursorX = x;
        for (const char* p = text; *p != '\0'; ++p)
        {
            const std::uint8_t* glyph = FindGlyph(*p);
            if (glyph != nullptr)
            {
                DrawGlyph(buffer, bufWidth, bufHeight, pixel, glyph, cursorX, yTop);
            }
            cursorX += CharAdvance;
        }
    }

private:
    // 每个字形 7 行，每行一个 5 位掩码（0x10 为最左列）
    static const std::uint8_t* FindGlyph(char c)
    {
        static const std::uint8_t glyphs[][GlyphHeight] = {
            // '0'
            {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E},
            // '1'
            {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
            // '2'
            {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},
            // '3'
            {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E},
            // '4'
            {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
            // '5'
            {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
            // '6'
            {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},
            // '7'
            {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
            // '8'
            {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
            // '9'
            {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
            // 'F'
            {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10},
            // 'P'
            {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
            // 'S'
            {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E},
            // ':'
            {0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00},
        };

        if (c >= '0' && c <= '9') return glyphs[c - '0'];
        switch (c)
        {
            case 'F': return glyphs[10];
            case 'P': return glyphs[11];
            case 'S': return glyphs[12];
            case ':': return glyphs[13];
            default:  return nullptr;   // 空格及其他不支持的字符跳过
        }
    }

    static void DrawGlyph(std::uint32_t* buffer, int bufWidth, int bufHeight,
                          std::uint32_t pixel, const std::uint8_t* glyph,
                          int x, int yTop)
    {
        for (int row = 0; row < GlyphHeight; ++row)
        {
            std::uint8_t bits = glyph[row];
            if (bits == 0) continue;
            for (int col = 0; col < GlyphWidth; ++col)
            {
                if ((bits & (0x10 >> col)) == 0) continue;
                FillBlock(buffer, bufWidth, bufHeight, pixel,
                          x + col * Scale, yTop - row * Scale);
            }
        }
    }

    // 以 (x, yTop) 为左上角填充 Scale x Scale 像素块（Y 向上：块内向行号减小方向扩展）
    static void FillBlock(std::uint32_t* buffer, int bufWidth, int bufHeight,
                          std::uint32_t pixel, int x, int yTop)
    {
        for (int sy = 0; sy < Scale; ++sy)
        {
            int y = yTop - sy;
            if (y < 0 || y >= bufHeight) continue;
            for (int sx = 0; sx < Scale; ++sx)
            {
                int px = x + sx;
                if (px < 0 || px >= bufWidth) continue;
                buffer[y * bufWidth + px] = pixel;
            }
        }
    }
};

#endif
