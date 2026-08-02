#include "Window.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#define MINIFB_AVOID_CPP_HEADERS
#include <MiniFB.h>

namespace
{
    // Key -> MiniFB 键码映射
    int ToMiniFBKey(Key key)
    {
        switch (key)
        {
            case Key::W:         return MFB_KB_KEY_W;
            case Key::A:         return MFB_KB_KEY_A;
            case Key::S:         return MFB_KB_KEY_S;
            case Key::D:         return MFB_KB_KEY_D;
            case Key::Q:         return MFB_KB_KEY_Q;
            case Key::E:         return MFB_KB_KEY_E;
            case Key::Space:     return MFB_KB_KEY_SPACE;
            case Key::LeftShift: return MFB_KB_KEY_LEFT_SHIFT;
            case Key::Escape:    return MFB_KB_KEY_ESCAPE;
            default:             return -1;
        }
    }

    int ToMiniFBMouseButton(MouseButton button)
    {
        switch (button)
        {
            case MouseButton::Left:   return MFB_MOUSE_LEFT;
            case MouseButton::Right:  return MFB_MOUSE_RIGHT;
            case MouseButton::Middle: return MFB_MOUSE_MIDDLE;
            default:                  return -1;
        }
    }

    double NowSeconds()
    {
        using Clock = std::chrono::steady_clock;
        static const Clock::time_point start = Clock::now();
        return std::chrono::duration<double>(Clock::now() - start).count();
    }
}

Window::~Window()
{
    Destroy();
}

bool Window::Create(const std::string& title, int w, int h)
{
    Destroy();

    handle = mfb_open_ex(title.c_str(),
                         static_cast<unsigned>(w),
                         static_cast<unsigned>(h),
                         0);
    if (handle == nullptr)
    {
        std::cerr << "Window create failed!" << std::endl;
        return false;
    }

    width = w;
    height = h;
    return true;
}

void Window::Destroy()
{
    if (handle != nullptr)
    {
        mfb_close(handle);
        handle = nullptr;
    }
}

bool Window::PollEvents()
{
    if (handle == nullptr) return false;

    if (mfb_update_events(handle) != MFB_STATE_OK)
    {
        handle = nullptr;   // MiniFB 在返回非 OK 时已销毁窗口
        return false;
    }

    const std::uint8_t* keyBuffer = mfb_get_key_buffer(handle);
    if (keyBuffer != nullptr)
    {
        for (int i = 0; i < static_cast<int>(Key::Count); i++)
        {
            int code = ToMiniFBKey(static_cast<Key>(i));
            input.keys[i] = (code >= 0) && (keyBuffer[code] != 0);
        }
    }

    const std::uint8_t* buttonBuffer = mfb_get_mouse_button_buffer(handle);
    if (buttonBuffer != nullptr)
    {
        for (int i = 0; i < static_cast<int>(MouseButton::Count); i++)
        {
            int code = ToMiniFBMouseButton(static_cast<MouseButton>(i));
            input.mouseButtons[i] = (code >= 0) && (buttonBuffer[code] != 0);
        }
    }

    float mouseX = static_cast<float>(mfb_get_mouse_x(handle));
    float mouseY = static_cast<float>(mfb_get_mouse_y(handle));

    if (hasLastMousePos)
    {
        input.mouseDeltaX = mouseX - lastMouseX;
        input.mouseDeltaY = mouseY - lastMouseY;
    }
    else
    {
        input.mouseDeltaX = 0.0f;
        input.mouseDeltaY = 0.0f;
        hasLastMousePos = true;
    }

    lastMouseX = mouseX;
    lastMouseY = mouseY;
    input.mouseX = mouseX;
    input.mouseY = mouseY;

    return true;
}

bool Window::Present(const std::uint32_t* pixels, int w, int h)
{
    if (handle == nullptr || pixels == nullptr) return false;

    // 渲染器的帧缓冲以 Y 向上存放（第 0 行是画面底部），而窗口帧缓冲要求
    // 第 0 行是画面顶部，因此这里按行倒序拷贝完成垂直翻转
    // （等价于原先的 SDL_FlipSurface，但只在提交前做一次）。
    std::size_t rowCount = static_cast<std::size_t>(h);
    std::size_t rowPixels = static_cast<std::size_t>(w);

    if (flipBuffer.size() != rowCount * rowPixels)
    {
        flipBuffer.resize(rowCount * rowPixels);
    }

    for (std::size_t y = 0; y < rowCount; y++)
    {
        const std::uint32_t* src = pixels + (rowCount - 1 - y) * rowPixels;
        std::copy(src, src + rowPixels, flipBuffer.begin() + y * rowPixels);
    }

    mfb_update_state state = mfb_update_ex(handle,
                                           flipBuffer.data(),
                                           static_cast<unsigned>(w),
                                           static_cast<unsigned>(h));
    if (state != MFB_STATE_OK)
    {
        handle = nullptr;   // 窗口已被 MiniFB 销毁
        return false;
    }

    return true;
}

float Window::DeltaTime()
{
    double now = NowSeconds();

    if (lastTimeSeconds < 0.0)
    {
        lastTimeSeconds = now;
        return 0.0f;
    }

    float dt = static_cast<float>(now - lastTimeSeconds);
    lastTimeSeconds = now;
    return dt;
}
