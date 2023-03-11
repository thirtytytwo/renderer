#ifndef __LYC_MATH_H__
#define __LYC_MATH_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef glm::vec2 vec2_t;
typedef glm::vec3 vec3_t;
typedef glm::vec4 vec4_t;
typedef glm::mat3 matrix3f;
typedef glm::mat4 matrix4f;
typedef glm::float32 f32;
typedef glm::int8 i8;


enum Projection_Type
{
    Ortho,
    Perspective
};
matrix4f Get_Model_Matrix(vec3_t rotation = vec3_t(0,0,0), vec3_t scale = vec3_t(1,1,1), vec3_t trans = vec3_t(0,0,0))
{
    matrix4f m;
    for(int i = 0; i < 3; i++)
    {
        if(rotation[i] == 0) continue;
        m = glm::rotate(m, rotation[i], i == 0 ? vec3_t(1,0,0) : (i == 1 ? vec3_t(0,1,0):vec3_t(0,0,1)));
    }
    m = glm::scale(m, scale);
    m = glm::translate(m, trans);
    return m;
}

matrix4f Get_View_Matrix(vec3_t rotation = vec3_t(0,0,0), vec3_t pos = vec3_t(0,0,0) )
{
    matrix4f v;
    for(int i = 0; i < 3; i++)
    {
        if(rotation[i] == 0) continue;
        v = glm::rotate(v, rotation[i], i == 0 ? vec3_t(1,0,0) : (i == 1 ? vec3_t(0,1,0):vec3_t(0,0,1)));
    }
    v = glm::translate(v, -pos);
    return v;
}

/// <summary>
/// 
/// </summary>
/// <param name="var1">field of view in perspective, left in ortho</param>
/// <param name="var2">aspect radio in perspective, right in ortho</param>
/// <param name="var3">z_near in perspective, bottom in ortho</param>
/// <param name="var4">z_far in perspective, top in ortho</param>
/// <param name="type">perspective or ortho</param>
/// <returns> matrix to clip space </returns>
matrix4f Get_Projection_Matrix(f32 var1, f32 var2, f32 var3, f32 var4, Projection_Type type)
{
    matrix4f ret;
    if(type == Projection_Type::Perspective)
    {
        //perspective projection
        ret = glm::perspective(glm::radians(var1), var2, var3, var4);
    }
    else
    {
        //ortho projection
        ret = glm::ortho(var1, var2, var3, var4);
    }
    return ret;
}
#endif
