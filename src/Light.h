#ifndef RENDERER_LIGHT_INCLUDE
#define RENDERER_LIGHT_INCLUDE

#include "Math.h"

class Light
{
public:
    Vec4f direction;
    Vec4f color;
    float intensity;

    Light()
        : direction(-0.5f, -1.0f, -0.3f, 0.0f)
        , color(1.0f, 1.0f, 1.0f, 1.0f)
        , intensity(1.0f)
    {
        direction = direction.normalized();
    }

    Vec4f GetDirection() const
    {
        return direction;
    }

    Vec4f GetColor() const
    {
        return color * intensity;
    }
};

#endif
