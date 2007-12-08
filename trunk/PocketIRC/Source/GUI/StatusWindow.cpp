#include "PocketIRC.h"
#include "StatusWindow.h"

#include "Config\Options.h"
#include "IrcString.h"
#include "StringUtil.h"

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

StatusWindow::StatusWindow()
{
	_TRACE("StatusWindow(0x%08X)::StatusWindow()", this);
}

StatusWindow::~StatusWindow()
{
	_TRACE("StatusWindow(0x%08X)::~StatusWindow()", this);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Creation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//	IDisplayWindow
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//	Window Procedure
/////////////////////////////////////////////////////////////////////////////

LRESULT StatusWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLEMSG(WM_COMMAND, OnCommand);
		default:
			return DisplayWindow::WndProc(msg, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//	Message Handling
/////////////////////////////////////////////////////////////////////////////

LRESULT StatusWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
	_TRACE("StatusWindow(0x%08X)::OnCreate()", this);

	DisplayWindow::OnCreate(wParam, lParam);

	m_hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_STATUSMENU));

	return 0;
}

void StatusWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	_TRACE("StatusWindow(0x%08X)::OnDestroy()", this);

	DestroyMenu(m_hMenu);
	m_hMenu = NULL;

	DisplayWindow::OnDestroy(wParam, lParam);
}

void StatusWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	_TRACE("StatusWindow(0x%08X)::OnCommand()", this);
	
	OnTabMenuCommand(LOWORD(wParam));
}

/////////////////////////////////////////////////////////////////////////////
//	ITabWindow
/////////////////////////////////////////////////////////////////////////////

bool StatusWindow::OnTabMenuCommand(UINT idCmd)
{
	_TRACE("StatusWindow(0x%08X)::OnTabMenuCommand(%u)", this, idCmd);
	
	tstring sInput = m_pMainWindow->GetInput();

	switch(idCmd)
	{
		case ID_SAY:
		case ID_STATUS_JOIN:
		case ID_STATUS_QUERY:
		{
			tstring sParam1 = GetWord(sInput, 0);
			if(sParam1.size())
			{
				if(IsChannel(sParam1) && idCmd != ID_STATUS_QUERY)
				{
					tstring sParam2 = GetWord(sInput, 1);

					m_pMainWindow->GetSession()->Join(sParam1, sParam2);
					m_pMainWindow->ClearInput();
				}
				else if(IsNick(sParam1) && idCmd != ID_STATUS_JOIN)
				{
					m_pMainWindow->OpenQuery(sParam1);
					m_pMainWindow->ClearInput();
				}
			}
		}
		break;
		case ID_STATUS_WHOIS:
		{
			tstring sParam1 = GetWord(sInput, 0);
			if(IsNick(sParam1))
			{
				m_pMainWindow->GetSession()->Whois(sParam1);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_STATUS_NICK:
		{
			tstring sParam1 = GetWord(sInput, 0);
			if(IsNick(sParam1))
			{
				m_pMainWindow->GetSession()->Nick(sParam1);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_STATUS_MODESELF:
		{
			m_pMainWindow->GetSession()->Mode(m_pMainWindow->GetSession()->GetNick(), sInput);
			m_pMainWindow->ClearInput();
		}
		break;
		case ID_STATUS_RAW:
		{
			if(sInput.size())
			{
				m_pMainWindow->GetSession()->Raw(sInput);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_STATUS_NICKSERV:
		{
			if(sInput.size())
			{
				tstring sMsg = _T("NICKSERV ");
				sMsg += sInput;
				m_pMainWindow->GetSession()->Raw(sMsg);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_STATUS_QUIT:
		{
			if(sInput.size() && g_Options.IsRegistered())
			{
				m_pMainWindow->GetSession()->Quit(sInput);
				m_pMainWindow->ClearInput();
			}
			else
			{
				m_pMainWindow->GetSession()->Quit(g_Options.GetQuitMsg());
				m_pMainWindow->ClearInput();
			}
		}
		break;
		default:
			return false;
	}
	
	return true;
}
