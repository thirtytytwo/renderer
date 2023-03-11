#ifndef __SCENEMANAGER_H__
#define __SCENEMANAGER_H__

#include <vector>
#include "Camera.h"
#include "IDriver.h"
#include "GameObject.h"

class SceneManager
{
private:
    std::vector<GameObject*> objs;
    IDriver* driver;
    Camera* main_cam;

    Render_Type type;
public:
    SceneManager(IDriver* _driver, Render_Type _type) :driver(_driver), type(_type) {}
    ~SceneManager() {}

    //TODO:在这里加上对obj中的idriver的设置
    void Add_GameObject(GameObject* objs);
    void Add_Camera(Camera* cam);
    void RenderAll();
};
#endif
