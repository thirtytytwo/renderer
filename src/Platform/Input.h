#ifndef RENDERER_PLATFORM_INPUT_INCLUDE
#define RENDERER_PLATFORM_INPUT_INCLUDE

// 平台无关的输入抽象：渲染器逻辑只依赖这里的 Key / InputState，
// 具体窗口后端负责把原生事件翻译过来。
enum class Key
{
    W,
    A,
    S,
    D,
    Q,
    E,
    Space,
    LeftShift,
    Escape,

    Count
};

enum class MouseButton
{
    Left,
    Right,
    Middle,

    Count
};

class InputState
{
public:
    bool IsKeyDown(Key key) const
    {
        return keys[static_cast<int>(key)];
    }

    bool IsMouseDown(MouseButton button) const
    {
        return mouseButtons[static_cast<int>(button)];
    }

    // 本帧鼠标位移（像素）
    float MouseDeltaX() const { return mouseDeltaX; }
    float MouseDeltaY() const { return mouseDeltaY; }

    float MouseX() const { return mouseX; }
    float MouseY() const { return mouseY; }

private:
    // 只有窗口后端可以写入状态
    friend class Window;

    bool  keys[static_cast<int>(Key::Count)] = {};
    bool  mouseButtons[static_cast<int>(MouseButton::Count)] = {};
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;
};

#endif
