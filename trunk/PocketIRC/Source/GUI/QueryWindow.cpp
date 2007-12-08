#include "PocketIRC.h"
#include "QueryWindow.h"

#include "IrcString.h"
#include "StringUtil.h"

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
//	IDisplayWindowFactory
/////////////////////////////////////////////////////////////////////////////

//IDisplayWindow* QueryWindowFactory::CreateDisplayWindow(const tstring& sKey, 
//	const tstring& sTitle, LPARAM lParam)
//{
//	_TRACE("QueryWindowFactory(0x%08X)::CreateDisplayWindow(\"%s\", \"%s\", 0x%08X)", this, sKey.c_str(), sTitle.c_str(), lParam);
//
//	if(IsNick(sKey))
//	{
//		QueryWindow* pWindow = new QueryWindow;
//
//	// Call order here is important
//		pWindow->SetQuery((IQuery*)lParam);
//		pWindow->SetMainWindow(m_pMainWindow);
//
//		pWindow->SetKey(sKey);
//		pWindow->SetTitle(sTitle);
//
//		pWindow->Create();
//
//		return pWindow;
//	}
//	return NULL;
//}


/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

QueryWindow::QueryWindow()
{
	_TRACE("QueryWindow(0x%08X)::QueryWindow()", this);
}

QueryWindow::~QueryWindow()
{
	_TRACE("QueryWindow(0x%08X)::~QueryWindow()", this);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Creation
/////////////////////////////////////////////////////////////////////////////

void QueryWindow::SetUser(const tstring& user)
{
	_TRACE("QueryWindow(0x%08X)::SetUser(\"%s\")", this, user.c_str());
	m_sUser = user;

	SetKey(m_sUser);
	SetTitle(m_sUser);
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

LRESULT QueryWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
	_TRACE("QueryWindow(0x%08X)::OnCreate()", this);
	DisplayWindow::OnCreate(wParam, lParam);

	m_hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_QUERYMENU));

	return 0;
}

void QueryWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	DestroyMenu(m_hMenu);
	m_hMenu = NULL;

	DisplayWindow::OnDestroy(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
//	ITabWindow
/////////////////////////////////////////////////////////////////////////////

bool QueryWindow::OnTabMenuCommand(UINT idCmd)
{
	tstring sInput = m_pMainWindow->GetInput();

	switch(idCmd)
	{
		case ID_SAY:
		case ID_QUERY_SAY:
		{
			if(sInput.size())
			{
				m_pMainWindow->GetSession()->PrivMsg(m_sUser, sInput);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_QUERY_ACT:
		{
			if(sInput.size())
			{
				m_pMainWindow->GetSession()->Action(m_sUser, sInput);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_QUERY_NOTICE:
		{
			if(sInput.size())
			{
				m_pMainWindow->GetSession()->Notice(m_sUser, sInput);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_QUERY_CTCP:
		{
			if(sInput.size())
			{
				m_pMainWindow->GetSession()->CTCP(m_sUser, GetWord(sInput, 0), GetWord(sInput, 1, true));
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_QUERY_INVITE:
		{
			if(IsChannel(sInput))
			{
				m_pMainWindow->GetSession()->Invite(m_sUser, sInput);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_QUERY_WHOIS:
		{
			m_pMainWindow->GetSession()->Whois(m_sUser);
		}
		break;
		case ID_QUERY_QUERY:
		{
			m_pMainWindow->OpenQuery(m_sUser); // wtf?
		}
		break;
		case ID_QUERY_CLOSE:
		{
			m_pMainWindow->CloseQuery(m_sUser);
		}
		break;
		case ID_QUERY_DCCSEND:
			m_pMainWindow->GetDCCHandler()->Send(m_sUser);
		break;
		case ID_QUERY_DCCCHAT:
			m_pMainWindow->GetDCCHandler()->Chat(m_sUser);
		break;
		default:
			return false;
	}

	return true;
}
