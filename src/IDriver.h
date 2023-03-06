#ifndef __I_DRIVER_H__
#define __I_DRIVER_H__

enum Render_Type
{
	Forward,
	Delay
};
class IDriver
{
public:
	//Lifetime func
	virtual bool Run() = 0;
	//virtual void yield() = 0;
	//virtual void Close() = 0;

	//render loop func
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	//render func
	//virtual void render() = 0;
};
#endif // !__I_DRIVER_H__
