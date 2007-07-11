#include "PocketIRC.h"
#include "InputBar.h"

#include "EditMenu.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

InputBar::InputBar()
{
	_TRACE("InputBar(0x%08X)::InputBar()", this);

	m_pfnDefEditProc = NULL;
	m_pfnDefMenuBarProc = NULL;
	m_hCmdParent = NULL;
	m_uID = 0;
	m_hCombo = NULL;
	m_hEdit = NULL;

	m_bLastSipState = false;
}


InputBar::~InputBar()
{
	_TRACE("InputBar(0x%08X)::~InputBar()", this);

	if(IsWindow(m_hCombo))
	{
		DestroyWindow(m_hCombo);
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Static stub proc
/////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK InputBar::EditStubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	InputBar* pInputBar = (InputBar*)GetWindowLong(hwnd, GWL_USERDATA);
	//_ASSERTE(pInputBar != NULL);

	return pInputBar->EditSubClassProc(msg, wParam, lParam);
}

LRESULT CALLBACK InputBar::MenuBarStubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// hack-o-rama.
	InputBar* pInputBar = (InputBar*)GetWindowLong(::GetWindow(GetDlgItem(hwnd, IDC_INPUTBOX), GW_CHILD), GWL_USERDATA);
	//_ASSERTE(pInputBar != NULL);

	return pInputBar->MenuBarSubClassProc(hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
//	Subclass proc
/////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK InputBar::EditSubClassProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CHAR:
			switch(wParam)
			{
			case VK_RETURN:
				{
					SendMessage(m_hCmdParent, WM_COMMAND, MAKELONG(m_uID, 0), 0);
				}
				return 0;
			}
		break;
		case WM_KEYDOWN:
			switch(wParam)
			{
			case VK_UP:
				{
					if(CB_ERR == SendMessage(m_hCombo, CB_GETCURSEL, 0, 0))
					{
						int count = SendMessage(m_hCombo, CB_GETCOUNT, 0, 0);
						if(count > 0)
						{
							SendMessage(m_hCombo, CB_SETCURSEL, count - 1, 0);
							return 0;
						}
					}
				}
			}
			break;
		case WM_LBUTTONDOWN:
		{
			SHRGINFO srgi = {sizeof(SHRGINFO), m_hEdit, {LOWORD(lParam), HIWORD(lParam)}, SHRG_RETURNCMD};
			if(SHRecognizeGesture(&srgi))
			{
				MapWindowPoints(m_hEdit, NULL, &srgi.ptDown, 1);
				TrackPopupEditMenu(m_hEdit, srgi.ptDown.x, srgi.ptDown.y, true);

				return 0;
			}
		}
		break;
	}
	_ASSERTE(m_pfnDefEditProc != NULL);
	return CallWindowProc(m_pfnDefEditProc, m_hEdit, msg, wParam, lParam);
}

LRESULT CALLBACK InputBar::MenuBarSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CLOSE:
		case WM_HELP:
			PostMessage(m_hCmdParent, msg, wParam, lParam);
		return 0;
		case WM_ACTIVATE:
			if(LOWORD(wParam) != WA_INACTIVE)
			{
				RestoreSipState();
				SetFocus(m_hEdit);
			}
		break;
		case WM_SETTINGCHANGE:
		{
			if(wParam == SPI_SETSIPINFO)
			{
				SIPINFO si = {sizeof(SIPINFO)};
				SipGetInfo(&si);

				m_bLastSipState = si.fdwFlags & SIPF_ON;
			}
		}
		break;
	}
	_ASSERTE(m_pfnDefMenuBarProc != NULL);
	return CallWindowProc(m_pfnDefMenuBarProc, hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

HRESULT InputBar::Create(HWND hMenuBar, HWND hCmdParent, UINT uID)
{
	_TRACE("InputBar(0x%08X)::Create(0x%08X, 0x%08X, 0x%08X)", this, hMenuBar, hCmdParent, uID);

	_ASSERTE(IsWindow(hMenuBar));
	_ASSERTE(IsWindow(hCmdParent));

	HRESULT hr = E_UNEXPECTED;

	m_hCmdParent = hCmdParent;
	m_uID = uID;

	// remove NOACTIVATE to receive input focus
	// remove TOOLWINDOW to receive smart minimise message
	SetWindowLong(hMenuBar, GWL_EXSTYLE, (GetWindowLong(hMenuBar, GWL_EXSTYLE) & ~(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW)));

	// HACK Guess at how big to make this :(
	m_hCombo = CommandBar_InsertComboBox(hMenuBar, GetModuleHandle(NULL), 
		GetSystemMetrics(SM_CXSCREEN) - (GetSystemMetrics(SM_CXSCREEN) / 3),
		WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL, IDC_INPUTBOX, 1);
	_ASSERTE(IsWindow(m_hCombo));

	if(!IsWindow(m_hCombo))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		// Center combo on menu bar
		RECT rcMenuClient;
		GetClientRect(hMenuBar, &rcMenuClient);

		RECT rcCombo;
		GetWindowRect(m_hCombo, &rcCombo);
		MapWindowPoints(NULL, hMenuBar, (LPPOINT)&rcCombo, 2);

		SetWindowPos(m_hCombo, HWND_TOP, rcCombo.left, ((rcMenuClient.bottom) - (rcCombo.bottom - rcCombo.top)) / 2, 0, 0, SWP_NOSIZE);

		// Subclass combobox edit control
		m_hEdit = ::GetWindow(m_hCombo, GW_CHILD);
		_ASSERTE(IsWindow(m_hEdit));

		SendMessage(m_hEdit, EM_LIMITTEXT, POCKETIRC_MAX_SAY_LEN, 0);

		SetWindowLong(m_hEdit, GWL_USERDATA, (LONG)this);
		m_pfnDefEditProc = (WNDPROC)SetWindowLong(m_hEdit, GWL_WNDPROC, (LONG)&InputBar::EditStubProc);
		_ASSERTE(m_pfnDefEditProc != NULL);

		hr = S_OK;
	}

	// Can't do this, it's used for something else... 
	// Use the value associated with the child edit instead
	//SetWindowLong(hMenuBar, GWL_USERDATA, (LONG)this);
	m_pfnDefMenuBarProc = (WNDPROC)SetWindowLong(hMenuBar, GWL_WNDPROC, (LONG)&InputBar::MenuBarStubProc);
	_ASSERTE(m_pfnDefMenuBarProc != NULL);

	return hr;
}

void InputBar::RestoreSipState()
{
	SipShowIM(m_bLastSipState ? SIPF_ON : SIPF_OFF);
}
