#ifndef __I_ROOT_H__
#define __I_ROOT_H__

#include "IDriver.h"
#include "SceneManager.h"

enum Platform
{
	Software,
	OpenGL
};
class IRoot
{
public:
	//get component
	virtual IDriver* GetDriver() = 0;
	virtual SceneManager* GetSceneManager() = 0;

	//get platform
	virtual Platform GetPlatform() = 0;
};

IRoot* CreateRoot(Platform plateform, int width, int height);
#endif //!__I_ROOT_H__