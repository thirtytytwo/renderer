#include "GameObject.h"
#include<assert.h>

#include "CShaderOpenGL.h"


GameObject::GameObject(Platform platform, const char* ver_path, const char* frag_path)
{
	if(platform == Platform::OpenGL)
	{
		this->shader = new CShaderOpenGL(ver_path,frag_path);
	}
	else
	{
		
	}
}

void GameObject::Render(matrix4f view, matrix4f projection)
{
	assert(model == nullptr);
	matrix4f m = Get_Model_Matrix(rotation, scale, pos);
	for(int i = 0; i < model->Get_Mesh_Size(); i++)
	{
		shader->SetMatrix(m, view,projection);
		shader->Draw(model->Get_Mesh(i));
	}
}


