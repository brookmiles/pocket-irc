#ifndef _STATUSWINDOW_H_INCLUDED_
#define _STATUSWINDOW_H_INCLUDED_

#include "Core\Session.h"

#include "DisplayWindow.h"

class Session;

class StatusWindow : 
	public DisplayWindow
{
public:
	DECL_WINDOW_CLASSNAME("PocketIRCStatusWnd");

	StatusWindow();
	~StatusWindow();

// ITabWindow
	TABWINDOWTYPE GetTabType() { return TABWINDOWTYPE_STATUS; };

private:
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	void OnDestroy(WPARAM wParam, LPARAM lParam);
	void OnCommand(WPARAM wParam, LPARAM lParam);

	bool OnTabMenuCommand(UINT idCmd);
};

#endif//_STATUSWINDOW_H_INCLUDED_
