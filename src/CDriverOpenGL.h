#ifndef __C_DRIVER_OPENGL_H__
#define __C_DRIVER_OPENGL_H__
#include "IDriver.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class CDriverOpenGL:public IDriver
{
public:
    CDriverOpenGL(int width, int height);
    ~CDriverOpenGL(){};

    //lifetime func
    virtual bool Run() override;
    //virtual void yield() override;
    //virtual void Close() override;

    //renderloop func
    virtual void BeginFrame() override;
    virtual void EndFrame() override;

    //render func
    //virtual void render() override;
    
private:
    GLFWwindow *window;
};
#endif // !__C_DRIVER_OPENGL_H__

