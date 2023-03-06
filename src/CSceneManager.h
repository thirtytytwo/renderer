#ifndef __C_SCENEMANAGER_H__
#define __C_SCENEMANAGER_H__

#include "ISceneManager.h"
#include "IGameObject.h"
#include "ICamera.h"

#include <list>

class CSceneManager : public ISceneManager
{
private:
    std::list<IGameObject*> gameobjects;
    ICamera* camera;

    IDriver* driver;
public:
    //TODO 定义构造函数，目前只是为了编译通过
    CSceneManager(IDriver* driver) {};
    ~CSceneManager(){};
    
    //virtual void RenderAll() override;
};

#endif // !__C_SCENEMANAGER_H__