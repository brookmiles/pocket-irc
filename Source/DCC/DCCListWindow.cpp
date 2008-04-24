#include "PocketIRC.h"
#include "DCCListWindow.h"

#include "Common\ListViewUtil.h"
#include "Common\WindowString.h"
#include "Common\IrcString.h"

#include "GUI\resource.h"

BOOL CALLBACK DCCListWindow::DummyDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			HWND hList = GetDlgItem(hwnd, IDC_DCCLIST_LIST);
			_ASSERTE(IsWindow(hList));

			ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);

			LVCOLUMN lvc = {LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH};

			lvc.cx = 145;
			lvc.pszText = _T("Connection");
			lvc.iSubItem = 0;
			ListView_InsertColumn(hList, 0, &lvc);

			lvc.cx = 70;
			lvc.pszText = _T("State");
			lvc.iSubItem = 1;
			ListView_InsertColumn(hList, 1, &lvc);
		}
		break;
		case WM_COMMAND:
			SendMessage(GetParent(hwnd), msg, wParam, lParam);
		break;
		case WM_NOTIFY:
			SendMessage(GetParent(hwnd), msg, wParam, lParam);
		break;
		default:
			return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

DCCListWindow::DCCListWindow()
{
	_TRACE("DCCListWindow(0x%08X)::DCCListWindow()", this);

	m_pMainWindow = NULL;
	m_iHighlight = 0;
	m_bActive = false;
	m_hMenu = NULL;

	m_pDCCHandler = NULL;
	m_hDlg = NULL;
}

DCCListWindow::~DCCListWindow()
{
	_TRACE("DCCListWindow(0x%08X)::~DCCListWindow()", this);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Creation
/////////////////////////////////////////////////////////////////////////////

HRESULT DCCListWindow::Create()
{
	_TRACE("DCCListWindow(0x%08X)::Create()", this);
	
	_ASSERTE(m_pMainWindow != NULL);

	HRESULT hr = Window::Create(m_pMainWindow->GetWindow(), _T("DCC"), WS_CHILD | WS_CLIPSIBLINGS, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT);
	_ASSERTE(SUCCEEDED(hr));

	m_pMainWindow->AddWindowTab(this);
	m_pMainWindow->SetWindowTabHighlight(this, m_iHighlight);

	return hr;
}

void DCCListWindow::SetMainWindow(MainWindow* pMainWindow)
{
	_TRACE("DCCListWindow(0x%08X)::SetMainWindow(0x%08X)", this, pMainWindow);
	m_pMainWindow = pMainWindow;
}

void DCCListWindow::SetDCCHandler(IDCCHandler* pDCCHandler)
{
	_TRACE("DCCListWindow(0x%08X)::SetDCCHandler(0x%08X)", this, pDCCHandler);
	m_pDCCHandler = pDCCHandler;
}

/////////////////////////////////////////////////////////////////////////////
//	IDCCListWindow
/////////////////////////////////////////////////////////////////////////////

int DCCListWindow::GetHighlight()
{
	return m_iHighlight;
}

void DCCListWindow::SetHighlight(int iHighlight)
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

void DCCListWindow::Close()
{
	_TRACE("DCCListWindow(0x%08X)::Close()", this);

	m_pDCCHandler->ShowDCCListWindow(false);
}

void DCCListWindow::AddSession(IDCCSession* pSession)
{
	_TRACE("DCCListWindow(0x%08X)::AddSession(0x%08X)", this, pSession);

	HWND hList = GetDlgItem(m_hDlg, IDC_DCCLIST_LIST);

	tstring sDesc = pSession->GetDescription();
	tstring sState = GetDCCStateString(pSession->GetState());

	LVITEM lvi = {LVIF_PARAM | LVIF_TEXT};

	lvi.iItem = ListView_GetItemCount(hList);
	lvi.iSubItem = 0;
	lvi.lParam = (LPARAM)pSession;
	lvi.pszText = const_cast<LPTSTR>(sDesc.c_str());

	int index = ListView_InsertItem(hList, &lvi);
	_ASSERTE(index != -1);

	ListView_SetItemText(hList, index, 1, const_cast<LPTSTR>(sState.c_str()));

	SetHighlight(2);
}

void DCCListWindow::UpdateSession(IDCCSession* pSession)
{
	_TRACE("DCCListWindow(0x%08X)::UpdateSession(0x%08X)", this, pSession);

	HWND hList = GetDlgItem(m_hDlg, IDC_DCCLIST_LIST);

	tstring sDesc = pSession->GetDescription();
	tstring sState = GetDCCStateString(pSession->GetState());

	int index = ListView_FindItemByParam(hList, (LPARAM)pSession);
	if(index >= 0)
	{
		ListView_SetItemText(hList, index, 1, const_cast<LPTSTR>(sState.c_str()));
		SetHighlight(1);
	}
}

void DCCListWindow::RemoveSession(IDCCSession* pSession)
{
	_TRACE("DCCListWindow(0x%08X)::RemoveSession(0x%08X)", this, pSession);

	HWND hList = GetDlgItem(m_hDlg, IDC_DCCLIST_LIST);

	tstring sDesc = pSession->GetDescription();
	tstring sState = GetDCCStateString(pSession->GetState());

	int index = ListView_FindItemByParam(hList, (LPARAM)pSession);
	if(index >= 0)
	{
		ListView_DeleteItem(hList, index);
	}
	SetHighlight(1);
}

/////////////////////////////////////////////////////////////////////////////
//	ITabWindow
/////////////////////////////////////////////////////////////////////////////

bool DCCListWindow::OnTabMenuCommand(UINT idCmd)
{
	switch(idCmd)
	{
		case ID_DCC_CLOSEDCCLIST:
			Close();
		break;
		default:
			return false;
	}
	return true;
}

void DCCListWindow::DoMenu(POINT pt)
{
	HMENU hSub = GetSubMenu(m_hMenu, 0);
	UINT id = TrackPopupMenuEx(hSub, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, 
		pt.x, pt.y, m_hwnd, NULL);
	if(id)
	{
		OnTabMenuCommand(id);
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Window Procedure
/////////////////////////////////////////////////////////////////////////////

LRESULT DCCListWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLEMSGRET(WM_CREATE, OnCreate);
		HANDLEMSG(WM_DESTROY, OnDestroy);
		HANDLEMSG(WM_SIZE, OnSize);
		HANDLEMSG(WM_ACTIVATE, OnActivate);
		HANDLEMSG(WM_COMMAND, OnCommand);
		HANDLEMSG(WM_NOTIFY, OnNotify);
		HANDLEMSG(WM_INITMENUPOPUP, OnInitMenuPopup);
		default:
			return DefWindowProc(m_hwnd, msg, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//	Message Handling
/////////////////////////////////////////////////////////////////////////////

LRESULT DCCListWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
	_TRACE("DCCListWindow(0x%08X)::OnCreate()", this);

	m_hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_DCCMENU));
	_ASSERTE(m_hMenu != NULL);

	m_hDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DCCLIST), m_hwnd, &DCCListWindow::DummyDlgProc);
	_ASSERTE(IsWindow(m_hDlg));

	_ASSERTE(m_pDCCHandler != NULL);
	UINT nSessions = m_pDCCHandler->GetSessionCount();
	for(UINT i = 0; i < nSessions; ++i)
	{
		IDCCSession* pSession = m_pDCCHandler->GetSession(i);
		_ASSERTE(pSession != NULL);

		AddSession(pSession);
	}

	return 0;
}

void DCCListWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	_TRACE("DCCListWindow(0x%08X)::OnDestroy()", this);

	DestroyMenu(m_hMenu);

	_ASSERTE(m_pMainWindow != NULL);
	m_pMainWindow->RemoveWindowTab(this);

	Window::OnDestroy(wParam, lParam);
	delete this;
}

void DCCListWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	_TRACE("DCCListWindow(0x%08X)::OnSize()", this);

	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);

	SetWindowPos(m_hDlg, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);

	RECT rcDlg;
	GetClientRect(m_hDlg, &rcDlg);

	HWND hList = GetDlgItem(m_hDlg, IDC_DCCLIST_LIST);
	_ASSERTE(IsWindow(hList));

	RECT rcList;
	GetWindowRect(hList, &rcList);
	MapWindowPoints(NULL, m_hDlg, (POINT*)&rcList, 2);

	SetWindowPos(hList, NULL, 0, 0, rcDlg.right - rcList.left * 2, rcDlg.bottom - rcList.top - rcList.left, SWP_NOZORDER | SWP_NOMOVE);
}

void DCCListWindow::OnActivate(WPARAM wParam, LPARAM lParam)
{
	_TRACE("DCCListWindow(0x%08X)::OnActivate()", this);

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
			m_pMainWindow->SetCurrentTarget(_T(""));
		}
		break;
		case WA_INACTIVE:
			m_bActive = false;
		break;
	}
}

void DCCListWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
		case IDC_DCCLIST_ACCEPT:
		case ID_DCCLIST_ACCEPT:
			OnAccept();
		break;
		case IDC_DCCLIST_CLOSE:
		case ID_DCCLIST_CLOSE:
			OnClose(false);
		break;
		case IDC_DCCLIST_CLOSEALL:
		case ID_DCCLIST_CLOSEALL:
			OnClose(true);
		break;
		case IDC_DCCLIST_CLEAR:
		case ID_DCCLIST_CLEAR:
			OnClear();
		break;
		case IDC_DCCLIST_SEND:
			OnSend();
		break;
		case IDC_DCCLIST_CHAT:
			OnChat();
		break;
	}
}

void DCCListWindow::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* pnmh = (NMHDR*)lParam;
	switch(pnmh->code)
	{
		case GN_CONTEXTMENU:
		{
			NMRGINFO* pnmri = (NMRGINFO*)lParam;
			_ASSERTE(pnmri != NULL);

			switch(pnmri->hdr.idFrom)
			{
				case IDC_DCCLIST_LIST:
				{	
					HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_DCCLISTMENU));

					BOOL bTrackPopOk = TrackPopupMenuEx(GetSubMenu(hMenu, 0), TPM_BOTTOMALIGN | TPM_LEFTALIGN, 
						pnmri->ptAction.x, pnmri->ptAction.y, m_hwnd, NULL);
					_ASSERTE(bTrackPopOk);

					DestroyMenu(hMenu);
				}
				break;
			}
		}	
		break;
	}
}

void DCCListWindow::OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{

}

/////////////////////////////////////////////////////////////////////////////
//	Command Handling
/////////////////////////////////////////////////////////////////////////////

void DCCListWindow::OnAccept()
{
	HWND hList = GetDlgItem(m_hDlg, IDC_DCCLIST_LIST);

	int index = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	while(index != -1)
	{
		LPARAM lParam = ListView_GetItemParam(hList, index);

		IDCCSession* pSession = (IDCCSession*)lParam;
		_ASSERTE(pSession != NULL);

		if(pSession->GetState() == DCC_STATE_REQUEST)
		{
			pSession->Accept();
		}

		index = ListView_GetNextItem(hList, index, LVNI_SELECTED);
	}
}

void DCCListWindow::OnClose(bool bAll)
{
	HWND hList = GetDlgItem(m_hDlg, IDC_DCCLIST_LIST);

	int index = ListView_GetNextItem(hList, -1, bAll ? LVNI_ALL : LVNI_SELECTED);
	while(index != -1)
	{
		LPARAM lParam = ListView_GetItemParam(hList, index);

		IDCCSession* pSession = (IDCCSession*)lParam;
		_ASSERTE(pSession != NULL);

		ListView_DeleteItem(hList, index--);

		pSession->Close();

		index = ListView_GetNextItem(hList, index, bAll ? LVNI_ALL : LVNI_SELECTED);
	}
}

void DCCListWindow::OnClear()
{
	HWND hList = GetDlgItem(m_hDlg, IDC_DCCLIST_LIST);

	int index = ListView_GetNextItem(hList, -1, LVNI_ALL);
	while(index != -1)
	{
		LPARAM lParam = ListView_GetItemParam(hList, index);

		IDCCSession* pSession = (IDCCSession*)lParam;
		_ASSERTE(pSession != NULL);

		if(pSession->GetState() == DCC_STATE_ERROR || 
			pSession->GetState() == DCC_STATE_CLOSED || 
			pSession->GetState() == DCC_STATE_COMPLETE)
		{
			ListView_DeleteItem(hList, index--);
			m_pDCCHandler->RemoveSession(pSession);
		}

		index = ListView_GetNextItem(hList, index, LVNI_ALL);
	}
}

void DCCListWindow::OnChat()
{
	tstring sUser;

	if(GetDlgItemString(m_hDlg, IDC_DCCLIST_USER, sUser))
	{
		m_pDCCHandler->Chat(sUser);
	}
}

void DCCListWindow::OnSend()
{
	tstring sUser;

	if(GetDlgItemString(m_hDlg, IDC_DCCLIST_USER, sUser))
	{
		m_pDCCHandler->Send(sUser);
	}
}
