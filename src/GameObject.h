#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include "IRoot.h"
#include "LycMath.h"
#include "IShader.h"
#include "Model.h"

class GameObject
{
private:
    Model* model;
    IShader* shader;
public:
    vec3_t pos = vec3_t(0, 0, 0);
    vec3_t rotation = vec3_t(0, 0, 0);
    vec3_t scale = vec3_t(1, 1, 1);

    //TODO:最好还加一个初始化obj的位置等信息参数
    GameObject(Platform platform, const char* ver_path, const char* frag_path);
    ~GameObject() {}
    
    void Render(matrix4f view, matrix4f projection);
};

#endif // !__GAMEOBJECT_H__
