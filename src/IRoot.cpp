#include "IRoot.h"
#include "CRoot.h"

IRoot* CreateRoot(Platform platform, int width, int height)
{
    IRoot* root = nullptr;
    root = new CRoot(platform, width, height);

    return root;
}
