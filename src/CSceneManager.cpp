#include "CSceneManager.h"

CSceneManager::CSceneManager(IDriver* driver, Render_Type type)
{
    this->type = type;
    this->driver = driver;
}

void CSceneManager::Set_Camera(ICamera* cam)
{
    this->main_camera = cam;
}

void CSceneManager::Set_Light()
{
    
}

void CSceneManager::Add_GameObject(IGameObject* obj)
{
    
}


void CSceneManager::RenderAll()
{
    
}
