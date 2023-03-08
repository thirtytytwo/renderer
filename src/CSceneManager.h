#ifndef __C_SCENEMANAGER_H__
#define __C_SCENEMANAGER_H__

#include "ISceneManager.h"
#include "IGameObject.h"
#include "ICamera.h"
#include "IDriver.h"

#include <list>

class CSceneManager : public ISceneManager
{
private:
    std::list<IGameObject*> gameobjects;
    ICamera* main_camera;
    IDriver* driver;

    Render_Type type;
public:
    //TODO 定义构造函数，目前只是为了编译通过
    CSceneManager(IDriver* driver, Render_Type type);
    ~CSceneManager(){};
    
    virtual void RenderAll() override;
    //other func
    void Set_Camera(ICamera* cam);
    void Set_Light();
    void Add_GameObject(IGameObject* obj);
};

#endif // !__C_SCENEMANAGER_H__