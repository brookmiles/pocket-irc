#ifndef _QUERYWINDOW_H_INCLUDED_
#define _QUERYWINDOW_H_INCLUDED_

#include "Core\Session.h"

#include "DisplayWindow.h"

class QueryWindow : 
	public DisplayWindow
{
public:
	DECL_WINDOW_CLASSNAME("PocketIRCQueryWnd");

	QueryWindow();
	~QueryWindow();

	void SetUser(const tstring& user);

// ITabWindow
	TABWINDOWTYPE GetTabType() { return TABWINDOWTYPE_QUERY; };

private:
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	void OnDestroy(WPARAM wParam, LPARAM lParam);

	bool OnTabMenuCommand(UINT idCmd);
	
	tstring m_sUser;
};

#endif//_QUERYWINDOW_H_INCLUDED_
