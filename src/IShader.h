#ifndef __I_SHADER_H__
#define __I_SHADER_H__
#include "LycMath.h"
#include "Model.h"

class IShader
{
public:
    virtual void SetMatrix(matrix4f model, matrix4f view, matrix4f proj) = 0;
    virtual void Draw(Mesh &m) = 0;
};
#endif