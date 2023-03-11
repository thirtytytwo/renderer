#include "SceneManager.h"

void SceneManager::RenderAll()
{
    //pre render
    assert(main_cam == nullptr);
    matrix4f view = Get_View_Matrix(main_cam->pos, main_cam->rotation);
    //TODO：考虑将info结构体更改一下，成正交和透视投影都适用的结构体
    matrix4f projection = Get_Projection_Matrix(main_cam->info.field_of_view, main_cam->info.aspect_radio, main_cam->info.z_near, main_cam->info.z_far, main_cam->type);
    //forward render mode
    if(type == Render_Type::Forward)
    {
        // object render
        for(int i = 0; i < objs.size();i++)
        {
            objs[i]->Render(view, projection);
        }
    }
}
