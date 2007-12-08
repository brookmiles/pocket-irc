#ifndef _DCCCHATWINDOW_H_INCLUDED_
#define _DCCCHATWINDOW_H_INCLUDED_

#include "GUI\Window.h"
#include "GUI\MainWindow.h"
#include "GUI\DisplayWindow.h"
#include "Core\NetworkEvent.h"
#include "IDCCChat.h"

class DCCChatWindow :
	public DisplayWindow
{
public:
	DECL_WINDOW_CLASSNAME("PocketIRCDCCChatWnd");

	DCCChatWindow();
	~DCCChatWindow();

	void SetChat(IDCCChat* pChat);
	void SetDCCHandler(IDCCHandler* pDCCHandler) { m_pDCCHandler = pDCCHandler; }

	void OnEvent(NetworkEvent& event);

// ITabWindow (partially implemented by DisplayWindow)
	void DoDefaultAction();
	void DoMenu(POINT pt);
	TABWINDOWTYPE GetTabType() { return TABWINDOWTYPE_DCC; };

private:
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	void OnDestroy(WPARAM wParam, LPARAM lParam);

	bool OnTabMenuCommand(UINT idCmd);

	IDCCChat* m_pChat;
	IDCCHandler* m_pDCCHandler;
};

#endif//_DCCCHATWINDOW_H_INCLUDED_
