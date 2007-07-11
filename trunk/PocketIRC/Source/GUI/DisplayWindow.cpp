#include "PocketIRC.h"
#include "DisplayWindow.h"

#include "Common\IrcString.h"
#include "Config\Options.h"
#include "EventFormat.h"

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

DisplayWindow::DisplayWindow()
{
	_TRACE("DisplayWindow(0x%08X)::DisplayWindow()", this);

	m_pMainWindow = NULL;
	m_iHighlight = 0;
	m_bActive = false;
	m_hMenu = NULL;
}

DisplayWindow::~DisplayWindow()
{
	_TRACE("DisplayWindow(0x%08X)::~DisplayWindow()", this);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Creation
/////////////////////////////////////////////////////////////////////////////

HRESULT DisplayWindow::Create()
{
	_TRACE("DisplayWindow(0x%08X)::Create()", this);
	
	_ASSERTE(m_pMainWindow != NULL);

	HRESULT hr = Window::Create(m_pMainWindow->GetWindow(), m_sTitle.Str(), WS_CHILD | WS_CLIPSIBLINGS, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT);
	_ASSERTE(SUCCEEDED(hr));

	m_pMainWindow->AddWindowTab(this);
	m_pMainWindow->SetWindowTabHighlight(this, m_iHighlight);

	return hr;
}

void DisplayWindow::SetMainWindow(MainWindow* pMainWindow)
{
	_TRACE("DisplayWindow(0x%08X)::SetMainWindow(0x%08X)", this, pMainWindow);
	m_pMainWindow = pMainWindow;
}

/////////////////////////////////////////////////////////////////////////////
//	IDisplayWindow
/////////////////////////////////////////////////////////////////////////////

const String& DisplayWindow::GetKey()
{
	return m_sKey;
}

void DisplayWindow::SetKey(const String& sKey)
{
	m_sKey = sKey;
}

const String& DisplayWindow::GetTitle()
{
	return m_sTitle;
}

void DisplayWindow::SetTitle(const String& sTitle)
{
	m_sTitle = sTitle;

	if(m_pMainWindow != NULL)
	{
		m_pMainWindow->SetWindowTabTitle(this, sTitle);
	}
}

int DisplayWindow::GetHighlight()
{
	return m_iHighlight;
}

void DisplayWindow::SetHighlight(int iHighlight)
{
	if((!m_bActive && iHighlight > m_iHighlight) || (m_bActive && iHighlight == 0))
	{
		m_iHighlight = iHighlight;
		
		_ASSERTE(m_pMainWindow != NULL);
		if(m_pMainWindow)
		{
			m_pMainWindow->SetWindowTabHighlight(this, iHighlight);
		}
	}
}

void DisplayWindow::Print(const String& sText)
{
	m_View.AddLine(sText);
}

void DisplayWindow::PrintEvent(const DisplayEvent& displayEvent)
{
	SetHighlight(displayEvent.GetHighlight());
	Print(displayEvent.GetText());
}

void DisplayWindow::OnEvent(const NetworkEvent& networkEvent)
{
	DisplayEvent eventDisplay;

	Format* pFormat = g_Options.GetFormatList().GetFormat(networkEvent.GetEventID(), networkEvent.IsIncoming());
	_ASSERTE(pFormat != NULL);

	if(pFormat->IsEnabled())
	{
		DisplayEventFormat(eventDisplay, networkEvent, m_pMainWindow->GetSession()->GetNick());
		PrintEvent(eventDisplay);
	}
}

void DisplayWindow::Close()
{
	_TRACE("DisplayWindow(0x%08X)::Close()", this);

	_ASSERTE(m_pMainWindow != NULL);
	m_pMainWindow->RemoveWindowTab(this);

	DestroyWindow(m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////
//	ITabWindow
/////////////////////////////////////////////////////////////////////////////

void DisplayWindow::DoMenu(POINT pt)
{
	_TRACE("DisplayWindow(0x%08X)::DoMenu({%i, %i})", this, pt.x, pt.y);
	
	HMENU hSub = GetSubMenu(m_hMenu, 0);
	UINT id = TrackPopupMenuEx(hSub, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, 
		pt.x, pt.y, m_hwnd, NULL);
	if(id > 0)
	{
		OnTabMenuCommand(id);
	}
}

void DisplayWindow::DoDefaultAction()
{
	_TRACE("DisplayWindow(0x%08X)::DoDefaultAction()", this);

	OnTabMenuCommand(ID_SAY);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Procedure
/////////////////////////////////////////////////////////////////////////////

LRESULT DisplayWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLEMSGRET(WM_CREATE, OnCreate);
		HANDLEMSG(WM_DESTROY, OnDestroy);
		HANDLEMSG(WM_SIZE, OnSize);
		HANDLEMSG(WM_ACTIVATE, OnActivate);
		HANDLEMSGRET(WM_NOTIFY, OnNotify);
		default:
			return DefWindowProc(m_hwnd, msg, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//	Message Handling
/////////////////////////////////////////////////////////////////////////////

LRESULT DisplayWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
	_TRACE("DisplayWindow(0x%08X)::OnCreate()", this);

	HRESULT hr = m_View.Create(m_hwnd, IDC_VIEW, 0, 0, 240, 100);
	_ASSERTE(SUCCEEDED(hr));

	return 0;
}

void DisplayWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	_TRACE("DisplayWindow(0x%08X)::OnDestroy()", this);

	//Subclasses gotta do this if they load one
	//DestroyMenu(m_hMenu);

	Window::OnDestroy(wParam, lParam);
	delete this;
}

void DisplayWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	_TRACE("DisplayWindow(0x%08X)::OnSize()", this);

	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);

	MoveWindow(m_View.GetWindow(), 0, 0, rcClient.right, rcClient.bottom, TRUE);
}

void DisplayWindow::OnActivate(WPARAM wParam, LPARAM lParam)
{
	_TRACE("DisplayWindow(0x%08X)::OnActivate()", this);

	switch(wParam)
	{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
		{
			m_bActive = true;
			m_iHighlight = 0;

			_ASSERTE(m_pMainWindow != NULL);
			m_pMainWindow->SetWindowTabHighlight(this, 0);
			m_pMainWindow->SetWindowTabMenu(m_hMenu);
			m_pMainWindow->SetCurrentTarget(GetKey());
		}
		break;
		case WA_INACTIVE:
			m_bActive = false;
		break;
	}
}

LRESULT DisplayWindow::OnNotify(WPARAM wParam, LPARAM lParam)
{
	_TRACE("DisplayWindow(0x%08X)::OnNotify()", this);

	NMHDR* pnmh = (NMHDR*)lParam;
	switch(pnmh->code)
	{
		case NM_CLICK:
			_TRACE("...NM_CLICK ID(%d)", pnmh->idFrom);
			switch(pnmh->idFrom)
			{
				case IDC_VIEW:
				{
					_TRACE("...IDC_VIEW");

					String str = m_View.GetSelectedWord();
					if(str.Size())
					{
						str.Append(' ');
						m_pMainWindow->InsertInput(str);
					}
				}
				break;
			}
		break;
		case GN_CONTEXTMENU:
		{
			NMRGINFO* pnmri = (NMRGINFO*)lParam;
			_ASSERTE(pnmri != NULL);

			_TRACE("...GN_CONTEXTMENU ID(%d)", pnmri->hdr.idFrom);

			switch(pnmri->hdr.idFrom)
			{
				case IDC_VIEW:
					_TRACE("...IDC_VIEW");
					
					POINT pt = pnmri->ptAction;
					MapWindowPoints(NULL, m_View.GetWindow(), &pt, 1);

					UINT iMsg;
					UINT iChar;
					if(m_View.HitTestMsg((WORD)pt.x, (WORD)pt.y, &iMsg, &iChar))
					{
						String str = m_View.GetWordAtChar(iMsg, iChar);
						_TRACE("...Word selected: \"%s\"", str.Str());

						// Modify pt in place, map to screen coords
						MapWindowPoints(m_hwnd, NULL, &pt, 1);

						if(IsChannel(str))
						{
							DoOffChannelMenu((WORD)pt.x, (WORD)pt.y, str);
						}
						else if(IsUrl(str))
						{
							DoUrlMenu((WORD)pt.x, (WORD)pt.y, str);
						}
						else
						{
							DoMenu(pt);
						}
					}
					else
					{
						DoMenu(pt);
					}
				return TRUE;
			}
		}
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//	Utilities
/////////////////////////////////////////////////////////////////////////////

void DisplayWindow::DoOffChannelMenu(WORD x, WORD y, const String& sChannel)
{
	HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_OFFCHANNELMENU));
	_ASSERTE(hMenu != NULL);

	UINT iCmd = TrackPopupMenuEx(GetSubMenu(hMenu, 0), TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, 
		x, y, m_hwnd, NULL);

	String sInput = m_pMainWindow->GetInput();

	switch(iCmd)
	{
		case ID_CHANNEL_JOIN:
		{
			if(sInput.Size() == 0)
			{
				FavouriteChannel* pFavChan = g_Options.GetFavouriteChannelList().FindChannel(sChannel);
				if(pFavChan)
				{
					sInput = pFavChan->Key;
				}
			}
			m_pMainWindow->GetSession()->Join(sChannel, sInput);
			m_pMainWindow->ClearInput();
		}
		break;
	}

	DestroyMenu(hMenu);
}

void DisplayWindow::DoUrlMenu(WORD x, WORD y, const String& sUrl)
{
	HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_URLMENU));
	_ASSERTE(hMenu != NULL);

	UINT iCmd = TrackPopupMenuEx(GetSubMenu(hMenu, 0), TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, 
		x, y, m_hwnd, NULL);

	switch(iCmd)
	{
		case ID_URL_BROWSE:
		{
			CreateProcess(_T("iexplore.exe"), sUrl.Str(), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);
		}
		break;
	}
}
