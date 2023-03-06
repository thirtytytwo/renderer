#include "CDriverOpenGL.h"

#include <glad/glad.h>
#include <iostream>

CDriverOpenGL::CDriverOpenGL(int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Render v0.1", nullptr, nullptr);
    
    if(window == nullptr)
    {
        std::cout<<"the window point is null"<<std::endl;
        glfwTerminate();
    }
    //glad ptr init
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to init glad" << std::endl;
    }
    glfwMakeContextCurrent(window);
    glViewport(0,0,width,height);
}

bool CDriverOpenGL::Run()
{
    //event listened
    //return current window state
    return !glfwWindowShouldClose(window);
}

void CDriverOpenGL::BeginFrame()
{
    glClearColor(0.f,0.f,0.f,1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void CDriverOpenGL::EndFrame()
{
    glfwSwapBuffers(window);
    glfwPollEvents();
}



