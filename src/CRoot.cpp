#include "CRoot.h"

#include "CDriverOpenGL.h"
#include "CSceneManager.h"

CRoot::CRoot(Platform platform, int width, int height)
{
    if(platform == Platform::OpenGL)
    {
        this->platform = platform;
        this->width = width;
        this->height = height;

        this->driver = new CDriverOpenGL(width, height);
        this->scene_manager = new CSceneManager(this->driver, Render_Type::Forward);
    }
    else if(platform ==Platform::Software)
    {
        //TODO 后续加上软件渲染器
#if _WIN32
#elif _APPLE_
#endif
    }
    else{}
}

IDriver* CRoot::GetDriver()
{
    return this->driver;
}
ISceneManager* CRoot::GetSceneManager()
{
    return this->scene_manager;
}


