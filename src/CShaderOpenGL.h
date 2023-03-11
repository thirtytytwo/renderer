#ifndef __C_SHADER_OPENGL_H__
#define __C_SHADER_OPENGL_H__

#include "IShader.h"

class CShaderOpenGL:public IShader
{
private:
    matrix4f m,v,p,mv,vp,mvp;

public:
    CShaderOpenGL(const char* vertex, const char* fragment);

    //设置变量进入shader中
    void SetInt();
    void SetFloat();
    //通用函数类
    void Draw(Mesh& m) override;
    void SetMatrix(matrix4f model, matrix4f view, matrix4f proj) override;
};

#endif