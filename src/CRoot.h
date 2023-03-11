#ifndef __C_ROOT_H__
#define __C_ROOT_H__
#include "IRoot.h"
#include "SceneManager.h"

class CRoot:public IRoot
{
public:
    CRoot(){};
    CRoot(Platform platform, int width, int height);
    ~CRoot(){};

    virtual IDriver* GetDriver() override;
    virtual SceneManager* GetSceneManager() override;
private:
    IDriver* driver;
    SceneManager* scene_manager;
    Platform platform;
    int width, height;
};
#endif