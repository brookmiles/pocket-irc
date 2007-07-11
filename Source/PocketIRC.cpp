// PocketIRC.cpp : Defines the entry point for the application.
//

#include "PocketIRC.h"
#include "Application.h"
#include "GUI\MainWindow.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	_TRACE("_tWinMain(0x%08X, 0x%08X, \"%s\", 0x%08X)", hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	if(MainWindow::ActivatePrevious())
	{
		return S_FALSE;
	}
	else
	{
		Application app;

		HRESULT hr = app.Init(hInstance, lpCmdLine, nCmdShow);
		_ASSERTE(SUCCEEDED(hr));

		if(SUCCEEDED(hr))
		{
			hr = app.Run();
		}

		HRESULT unhr = app.UnInit();
		_ASSERTE(SUCCEEDED(unhr));
		return hr;
	}
	return 0;
}

