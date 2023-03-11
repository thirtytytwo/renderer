#ifndef __I_CAMERA_H__
#define __I_CAMERA_H__
#include "LycMath.h"
struct Info
{
    f32 field_of_view;
    f32 aspect_radio;
    f32 z_near;
    f32 z_far;

    Info()
    {
        field_of_view = 90.f;
        aspect_radio = 1.f;
        z_near = 0.1f;
        z_far = 100.f;
    }
};
class Camera
{
public:
    Info info;
    Projection_Type type;
    vec3_t pos, rotation;
    Camera(Projection_Type type = Projection_Type::Perspective);
    Camera(Info _info, Projection_Type type = Projection_Type::Perspective);

    void UpdateCamera_Trans(vec3_t pos = vec3_t(0,0,1.f));
    void UpdateCamera_Rotate(vec3_t rotate = vec3_t(0,0,0));
};
#endif