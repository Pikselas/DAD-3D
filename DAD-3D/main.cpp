#include"Window.h"
#include"Canvas3D.h"

int WINAPI WinMain(HINSTANCE , HINSTANCE , LPSTR , int ) 
{
	Window wnd;
	Canvas3D c3d(wnd);
	while (wnd.IsOpen())
	{
		c3d.ClearCanvas();
		c3d.PresentOnScreen();
		wnd.ProcessWindowEvents();
	}
	return 0;
}