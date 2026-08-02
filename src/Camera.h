#ifndef RENDERER_CAMERA_INCLUDE
#define RENDERER_CAMERA_INCLUDE

#include "Math.h"
#include "Platform/Input.h"

class Camera
{
public:
    Vec4f position;
    float yaw;
    float pitch;
    float moveSpeed;
    float rotateSpeed;
    float fov;
    float nearPlane;
    float farPlane;

    Camera()
        : position(0.0f, 0.0f, 5.0f, 1.0f)
        , yaw(-Math::PI * 0.5f)
        , pitch(0.0f)
        , moveSpeed(2.5f)
        , rotateSpeed(0.003f)
        , fov(Math::PI / 3.0f)
        , nearPlane(0.1f)
        , farPlane(100.0f)
    {
    }

    Vec4f GetForward() const
    {
        return Vec4f(
            std::cos(pitch) * std::cos(yaw),
            std::sin(pitch),
            std::cos(pitch) * std::sin(yaw),
            0.0f
        ).normalized();
    }

    Vec4f GetRight() const
    {
        Vec4f worldUp(0.0f, 1.0f, 0.0f, 0.0f);
        return GetForward().cross(worldUp).normalized();
    }

    Vec4f GetUp() const
    {
        return GetRight().cross(GetForward());
    }

    Mat4 GetViewMatrix() const
    {
        Vec4f forward = GetForward();
        Vec4f target = position + forward;
        Vec4f up = GetUp();
        return Mat4::LookAt(position, target, up);
    }

    Mat4 GetProjectionMatrix(float aspect) const
    {
        return Mat4::Perspective(fov, aspect, nearPlane, farPlane);
    }

    void ProcessKeyboard(const InputState& input, float dt)
    {
        Vec4f forward = GetForward();
        forward.w = 0.0f;
        Vec4f right = GetRight();
        right.w = 0.0f;

        float speed = moveSpeed * dt;

        if (input.IsKeyDown(Key::W))
            position = position + forward * speed;
        if (input.IsKeyDown(Key::S))
            position = position - forward * speed;
        if (input.IsKeyDown(Key::A))
            position = position - right * speed;
        if (input.IsKeyDown(Key::D))
            position = position + right * speed;
    }

    void ProcessMouse(float dx, float dy)
    {
        yaw += dx * rotateSpeed;
        pitch -= dy * rotateSpeed;

        float maxPitch = Math::PI * 0.49f;
        if (pitch > maxPitch) pitch = maxPitch;
        if (pitch < -maxPitch) pitch = -maxPitch;
    }
};

#endif
