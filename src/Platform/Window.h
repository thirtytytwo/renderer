#ifndef RENDERER_PLATFORM_WINDOW_INCLUDE
#define RENDERER_PLATFORM_WINDOW_INCLUDE

#include <cstdint>
#include <string>
#include <vector>

#include "Input.h"
#include "PixelFormat.h"

struct mfb_window;

// 基于 MiniFB 的窗口 / 事件 / 帧缓冲呈现封装。
// 对上层暴露轮询式接口，屏蔽后端细节；更换窗口后端只需重写本类的实现。
class Window
{
public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // 创建窗口，失败返回 false
    bool Create(const std::string& title, int width, int height);

    // 抽取本帧输入事件；窗口被关闭时返回 false
    bool PollEvents();

    // 把 32 位帧缓冲呈现到窗口；失败或用户关闭窗口时返回 false。
    // 传入缓冲按 Y 向上存放（第 0 行为画面底部），内部会做垂直翻转。
    bool Present(const std::uint32_t* pixels, int width, int height);

    // 帧缓冲应使用的像素打包格式（桌面平台原生为 ARGB8888）
    static PixelFormat Format() { return PixelFormat::ARGB8888; }

    const InputState& Input() const { return input; }

    int Width() const { return width; }
    int Height() const { return height; }

    // 距上一次调用的间隔（秒）
    float DeltaTime();

private:
    void Destroy();

    mfb_window* handle = nullptr;
    InputState  input;
    int         width = 0;
    int         height = 0;

    // Present 时做垂直翻转用的暂存缓冲，避免每帧重新分配
    std::vector<std::uint32_t> flipBuffer;

    bool        hasLastMousePos = false;
    float       lastMouseX = 0.0f;
    float       lastMouseY = 0.0f;
    double      lastTimeSeconds = -1.0;
};

#endif
