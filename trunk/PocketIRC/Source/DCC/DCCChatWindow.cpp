#include "PocketIRC.h"
#include "DCCChatWindow.h"

#include "GUI\resource.h"
#include "GUI\EventFormat.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

DCCChatWindow::DCCChatWindow()
{
	_TRACE("DCCChatWindow(0x%08X)::DCCChatWindow()", this);

	m_pChat = NULL;
	m_pDCCHandler = NULL;
}	

DCCChatWindow::~DCCChatWindow()
{
	_TRACE("DCCChatWindow(0x%08X)::~DCCChatWindow()", this);
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

void DCCChatWindow::SetChat(IDCCChat* pChat)
{
	_TRACE("DCCChatWindow(0x%08X)::SetChat(0x%08X)", this, pChat);
	m_pChat = pChat;
}

void DCCChatWindow::OnEvent(NetworkEvent& event)
{
	DisplayEvent disp;
	DisplayEventFormat(disp, event, m_pDCCHandler->GetNick());

	PrintEvent(disp);
}

/////////////////////////////////////////////////////////////////////////////
//	IDisplayWindow
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//	Window Procedure
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//	Message Handling
/////////////////////////////////////////////////////////////////////////////

LRESULT DCCChatWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
	_TRACE("DCCChatWindow(0x%08X)::OnCreate()", this);
	DisplayWindow::OnCreate(wParam, lParam);

	m_hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_DCCCHATMENU));
	_ASSERTE(m_hMenu != NULL);

	return 0;
}

void DCCChatWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	DestroyMenu(m_hMenu);
	m_hMenu = NULL;

	DisplayWindow::OnDestroy(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
//	ITabWindow
/////////////////////////////////////////////////////////////////////////////

bool DCCChatWindow::OnTabMenuCommand(UINT idCmd)
{
	switch(idCmd)
	{
		case ID_SAY:
		case ID_DCCCHAT_SAY:
		{
			String str = m_pMainWindow->GetInput();
			m_pChat->Say(str);
			m_pMainWindow->ClearInput();
		}
		break;
		case ID_DCCCHAT_ACT:
		{
			String str = m_pMainWindow->GetInput();
			m_pChat->Act(str);
			m_pMainWindow->ClearInput();
		}
		break;
		case ID_DCCCHAT_CLOSE:
		{
			m_pChat->CloseChat();
		}
		break;
		case ID_DCCCHAT_DCCSEND:
			m_pDCCHandler->Send(m_pChat->GetRemoteUser());
		break;
	}
	return true;
}

void DCCChatWindow::DoMenu(POINT pt)
{
	HMENU hSub = GetSubMenu(m_hMenu, 0);
	UINT id = TrackPopupMenuEx(hSub, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, 
		pt.x, pt.y, m_hwnd, NULL);
	if(id)
	{
		OnTabMenuCommand(id);
	}
}

void DCCChatWindow::DoDefaultAction()
{
	OnTabMenuCommand(ID_SAY);
}
