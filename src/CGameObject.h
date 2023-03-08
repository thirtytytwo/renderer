#ifndef __C_GAMEOBJECT_H__
#define __C_GAMEOBJECT_H__

#include "IGameObject.h"
#include "LycMath.h"
#include "Model.h"

class CGameObject :public IGameObject
{
private:
    vec3_t position;
    vec3_t rotation;

    //components
    Model* model;
public:
    CGameObject();
    ~CGameObject();

    void SetModel();
    void render() override;
};
#endif