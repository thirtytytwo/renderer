#include "ISceneManager.h"
#include "IRoot.h"
#include "IDriver.h"

int main()
{
    IRoot* root = CreateRoot(Platform::OpenGL, 1024, 1024);

    IDriver* driver = root->GetDriver();
    ISceneManager* scene_manager = root->GetSceneManager();
}