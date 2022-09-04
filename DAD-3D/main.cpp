#include"Window.h"

int WINAPI WinMain(HINSTANCE , HINSTANCE , LPSTR , int ) 
{
	Window wnd;
	while (wnd.IsOpen())
	{
		wnd.ProcessWindowEvents();
	}
	return 0;
}