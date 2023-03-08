#include "ISceneManager.h"
#include "IRoot.h"
#include "IDriver.h"
#include "Model.h"
int main()
{
    // IRoot* root = CreateRoot(Platform::OpenGL, 1024, 1024);
    //
    // IDriver* driver = root->GetDriver();
    // ISceneManager* scene_manager = root->GetSceneManager();

    Model *model = new Model("D:/renderer/assets/model/african_head.obj");
    std::cout << "01" << std::endl;
    return 0;
}