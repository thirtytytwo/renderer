#include "CShaderOpenGL.h"

CShaderOpenGL::CShaderOpenGL(const char* vertex, const char* fragment)
{
    
}

void CShaderOpenGL::SetMatrix(matrix4f model, matrix4f view, matrix4f proj)
{
    m = model;
    v = view;
    p = proj;
    mv = view * model;
    vp = proj * view;
    mvp = proj * view * model;
}


