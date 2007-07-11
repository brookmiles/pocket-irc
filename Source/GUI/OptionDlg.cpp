#include "PocketIRC.h"
#include "OptionDlg.h"

#include "EditMenu.h"
#include "Formatter.h"
#include "EventFormat.h"
#include "Core\NetworkEvent.h"

#include "ChooseColor.h"
#include "Common\WindowString.h"
#include "Common\TabCtrlUtil.h"

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
//	Static Dummy Proc for Child Dialogs
/////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK OptionDlg::DummyProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_COMMAND:
		case WM_NOTIFY:
		case WM_MEASUREITEM:
		case WM_DRAWITEM:
			return SendMessage(GetParent(GetParent(hwnd)), msg, wParam, lParam);
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

OptionDlg::OptionDlg(Options *pOptions) : Dlg(GetModuleHandle(NULL), IDD_OPTIONS), m_hwndPane(NULL)
{
	_ASSERTE(pOptions != NULL);

	m_pOptions = pOptions;
}

OptionDlg::~OptionDlg()
{

}

/////////////////////////////////////////////////////////////////////////////
//	Internal Methods
/////////////////////////////////////////////////////////////////////////////

void OptionDlg::ActivateTab(bool bActivate)
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_OPTIONS_TAB);
	_ASSERTE(IsWindow(hTab));

	int index = TabCtrl_GetCurSel(hTab);
	if(index != -1)
	{
		TCITEM tci = {TCIF_PARAM};
		bool bGetItemOk = TabCtrl_GetItem(hTab, index, &tci) != 0;
		_ASSERTE(bGetItemOk != FALSE);

		HWND hDlg = (HWND)tci.lParam;
		_ASSERTE(IsWindow(hDlg));

		HWND hScroll = GetDlgItem(m_hwnd, IDC_OPTIONS_VSCROLL);
		_ASSERTE(IsWindow(hScroll));

		if(bActivate)
		{
			RECT rcClient;
			GetClientRect(m_hwnd, &rcClient);

			RECT rcTab;
			GetWindowRect(hTab, &rcTab);
			MapWindowPoints(NULL, m_hwnd, (LPPOINT)&rcTab, 2);

			RECT rcDlg;
			GetWindowRect(hDlg, &rcDlg);

			TabCtrl_AdjustRect(hTab, FALSE, &rcTab);

			bool showScroll = rcDlg.bottom - rcDlg.top > rcTab.bottom - rcTab.top;
			if(showScroll)
			{
				int scrollWidth = GetSystemMetrics(SM_CXVSCROLL);
				SetWindowPos(m_hwndPane, HWND_TOPMOST, 0, 0, rcClient.right - scrollWidth, rcTab.bottom, SWP_SHOWWINDOW);
				SetWindowPos(hScroll, HWND_TOPMOST, rcClient.right - scrollWidth, 0, scrollWidth, rcTab.bottom, SWP_SHOWWINDOW);

				SCROLLINFO si = {sizeof(SCROLLINFO)};
				si.fMask = SIF_ALL;
				si.nPage = (rcTab.bottom - rcTab.top);
				si.nMax = (rcDlg.bottom - rcDlg.top) - (rcTab.bottom - rcTab.top) + si.nPage;
				SetScrollInfo(hScroll, SB_CTL, &si, TRUE);
			}
			else
			{
				SetWindowPos(m_hwndPane, HWND_TOPMOST, 0, 0, rcClient.right, rcTab.bottom, SWP_SHOWWINDOW);
				SetWindowPos(hScroll, 0, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE);
			}

			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
		}
		else
		{
			ShowWindow(hDlg, SW_HIDE);
		}
	}
}

HWND OptionDlg::GetDlg(UINT idDlg)
{
	HWND hwnd = GetDlgItem(m_hwndPane, idDlg);
	_ASSERTE(IsWindow(hwnd));
	return hwnd;
}

Format* OptionDlg::GetCurFormatSel()
{
	HWND hDlgFormat = GetDlg(IDC_OPTIONS_FORMATDLG);

	Format* pFormat  = NULL;
	int index = SendDlgItemMessage(hDlgFormat, IDC_FORMAT_EVENT, CB_GETCURSEL, 0, 0);
	if(index != CB_ERR)
	{
		pFormat = (Format*)SendDlgItemMessage(hDlgFormat, IDC_FORMAT_EVENT, CB_GETITEMDATA, index, 0);
	}
	return pFormat;
}

/////////////////////////////////////////////////////////////////////////////
//	Message Handling
/////////////////////////////////////////////////////////////////////////////

BOOL OptionDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLEMSG(WM_INITDIALOG, OnInitDialog);
		HANDLEMSG(WM_COMMAND, OnCommand);
		HANDLEMSGRET(WM_NOTIFY, OnNotify);
		HANDLEMSG(WM_SIZE, OnSize);
		HANDLEMSG(WM_DRAWITEM, OnDrawItem);
		HANDLEMSG(WM_VSCROLL, OnVScroll);
		HANDLEMSG(WM_SETTINGCHANGE, OnSettingChange);
		default:
			return Dlg::DefProc(msg, wParam, lParam);
	}
	return TRUE;
}

int CALLBACK OptionDlg::EnumFontFamStub(ENUMLOGFONT FAR *lpelf, TEXTMETRIC FAR *lpntm, int FontType, LPARAM lParam)
{
	OptionDlg* pOptionDlg = (OptionDlg*)lParam;
	_ASSERTE(pOptionDlg != NULL);

	return pOptionDlg->EnumFontFamProc(lpelf, lpntm, FontType);
}

int OptionDlg::EnumFontFamProc(ENUMLOGFONT FAR *lpelf, TEXTMETRIC FAR *lpntm, int FontType)
{
	HWND hDlgFormat = GetDlg(IDC_OPTIONS_FORMATDLG);

	HWND hList = GetDlgItem(hDlgFormat, IDC_FORMAT_FONT);
	_ASSERTE(IsWindow(hList));

	int index = SendMessage(hList, CB_ADDSTRING, 0, (LPARAM)lpelf->elfLogFont.lfFaceName);
	_ASSERTE(index != CB_ERR);
	if(index != CB_ERR)
	{
		if(m_pOptions->GetFontName().Compare(lpelf->elfLogFont.lfFaceName, false))
		{
			SendMessage(hList, CB_SETCURSEL, index, 0);
		}
	}

	return TRUE;
}

void OptionDlg::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	Dlg::DefProc(WM_INITDIALOG, wParam, lParam);

	m_hwndPane = GetDlgItem(m_hwnd, IDC_OPTIONS_PANE);
	_ASSERTE(IsWindow(m_hwndPane));

	// Set up tab control
	HWND hTab = GetDlgItem(m_hwnd, IDC_OPTIONS_TAB);
	SendMessage(hTab, CCM_SETVERSION, COMCTL32_VERSION, 0);
	TabCtrl_FitToParent(hTab);

	// Create Sub-Dialogs
	HWND hDlgServer = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPTIONS_SERVER),
		m_hwndPane, &OptionDlg::DummyProc);
	SetWindowLong(hDlgServer, GWL_ID, IDC_OPTIONS_SERVERDLG);

	HWND hDlgDisplay = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPTIONS_DISPLAY),
		m_hwndPane, &OptionDlg::DummyProc);
	SetWindowLong(hDlgDisplay, GWL_ID, IDC_OPTIONS_DISPLAYDLG);

	HWND hDlgFormat = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPTIONS_FORMAT),
		m_hwndPane, &OptionDlg::DummyProc);
	SetWindowLong(hDlgFormat, GWL_ID, IDC_OPTIONS_FORMATDLG);

	HWND hDlgIdent = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPTIONS_IDENT),
		m_hwndPane, &OptionDlg::DummyProc);
	SetWindowLong(hDlgIdent, GWL_ID, IDC_OPTIONS_IDENTDLG);

	HWND hDlgDCC = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPTIONS_DCC),
		m_hwndPane, &OptionDlg::DummyProc);
	SetWindowLong(hDlgDCC, GWL_ID, IDC_OPTIONS_DCCDLG);

	HWND hDlgAbout = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPTIONS_ABOUT),
		m_hwndPane, &OptionDlg::DummyProc);
	SetWindowLong(hDlgAbout, GWL_ID, IDC_OPTIONS_ABOUTDLG);

	// Add tabs
	TCITEM tci = {TCIF_TEXT | TCIF_PARAM};
	int tabcount = 0;

	tci.pszText = _T("Server");
	tci.lParam = (LPARAM)hDlgServer;
	int iInsertTabOk = TabCtrl_InsertItem(hTab, tabcount++, &tci);
	_ASSERTE(iInsertTabOk != -1);

	tci.pszText = _T("Display");
	tci.lParam = (LPARAM)hDlgDisplay;
	iInsertTabOk = TabCtrl_InsertItem(hTab, tabcount++, &tci);
	_ASSERTE(iInsertTabOk != -1);

	tci.pszText = _T("Format");
	tci.lParam = (LPARAM)hDlgFormat;
	iInsertTabOk = TabCtrl_InsertItem(hTab, tabcount++, &tci);
	_ASSERTE(iInsertTabOk != -1);

	tci.pszText = _T("Ident");
	tci.lParam = (LPARAM)hDlgIdent;
	iInsertTabOk = TabCtrl_InsertItem(hTab, tabcount++, &tci);
	_ASSERTE(iInsertTabOk != -1);

	tci.pszText = _T("DCC");
	tci.lParam = (LPARAM)hDlgDCC;
	iInsertTabOk = TabCtrl_InsertItem(hTab, tabcount++, &tci);
	_ASSERTE(iInsertTabOk != -1);

	tci.pszText = _T("About");
	tci.lParam = (LPARAM)hDlgAbout;
	iInsertTabOk = TabCtrl_InsertItem(hTab, tabcount++, &tci);
	_ASSERTE(iInsertTabOk != -1);

	// Init Server Dialog

	SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FONT), FALSE);

	if(!m_pOptions->IsRegistered())
	{
		EnableWindow(GetDlgItem(hDlgServer, IDC_OPTIONS_REALNAME), FALSE);
		EnableWindow(GetDlgItem(hDlgServer, IDC_OPTIONS_QUITMSG), FALSE);
	}

	SendDlgItemMessage(hDlgServer, IDC_OPTIONS_NICKNAME, EM_LIMITTEXT, POCKETIRC_MAX_NICK_LEN, 0);
	SetDlgItemText(hDlgServer, IDC_OPTIONS_NICKNAME, m_pOptions->GetNick().Str());

	SetEditMenuSubclass(GetDlgItem(hDlgServer, IDC_OPTIONS_NICKNAME), false);

	SendDlgItemMessage(hDlgServer, IDC_OPTIONS_REALNAME, EM_LIMITTEXT, POCKETIRC_MAX_REALNAME_LEN, 0);
	SetDlgItemText(hDlgServer, IDC_OPTIONS_REALNAME, m_pOptions->GetRealName().Str());

	SetEditMenuSubclass(GetDlgItem(hDlgServer, IDC_OPTIONS_REALNAME), true);

	SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERPORT, EM_LIMITTEXT, 5, 0);
	SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, EM_LIMITTEXT, POCKETIRC_MAX_HOST_NAME_LEN, 0);
	SetDlgItemInt(hDlgServer, IDC_OPTIONS_SERVERPORT, POCKETIRC_DEFAULT_PORT, FALSE);

	// Server name is combo so subclass child edit
	SetEditMenuSubclass(GetWindow(GetDlgItem(hDlgServer, IDC_OPTIONS_SERVERNAME), GW_CHILD), false);
	SetEditMenuSubclass(GetDlgItem(hDlgServer, IDC_OPTIONS_SERVERPORT), false);

	SendDlgItemMessage(hDlgServer, IDC_OPTIONS_QUITMSG, EM_LIMITTEXT, POCKETIRC_MAX_IRC_LINE_LEN, 0);
	SetDlgItemText(hDlgServer, IDC_OPTIONS_QUITMSG, m_pOptions->GetQuitMsg().Str());

	SetEditMenuSubclass(GetDlgItem(hDlgServer, IDC_OPTIONS_QUITMSG), true);

	Host* pHost = NULL;
	HostList& lstHosts = m_pOptions->GetHostList();

	for(UINT i = 0; i < lstHosts.Count(); ++i)
	{
		pHost = NULL;
		HRESULT hr = lstHosts.Item(i, &pHost);
		_ASSERTE(SUCCEEDED(hr));
		_ASSERTE(pHost != NULL);

		int index = SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, CB_ADDSTRING, 0, (LPARAM)pHost->GetAddress().Str());
		_ASSERTE(index != CB_ERR);
		if(index != CB_ERR)
		{
			SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, CB_SETITEMDATA, index, (LPARAM)pHost);

			if(pHost == lstHosts.GetDefault() || (lstHosts.GetDefault() == 0 && index == 0))
			{
				SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, CB_SETCURSEL, index, 0);
				SetDlgItemInt(hDlgServer, IDC_OPTIONS_SERVERPORT, pHost->GetPort(), FALSE);
				SetDlgItemText(hDlgServer, IDC_OPTIONS_SERVERPASS, pHost->GetPass().Str());
			}
		}
	}


	// Init Display Dialog
	SendDlgItemMessage(hDlgDisplay, IDC_DISPLAY_BACKGROUND, EM_LIMITTEXT, MAX_PATH, 0);
	SetDlgItemText(hDlgDisplay, IDC_DISPLAY_BACKGROUND, m_pOptions->GetBackImage().Str());

	SetEditMenuSubclass(GetDlgItem(hDlgDisplay, IDC_DISPLAY_BACKGROUND), false);

	SendDlgItemMessage(hDlgDisplay, m_pOptions->GetBackAlignTop() ? IDC_DISPLAY_BACKALIGNTOP : IDC_DISPLAY_BACKALIGNBOTTOM, BM_SETCHECK, BST_CHECKED, 0);

	SendDlgItemMessage(hDlgDisplay, IDC_DISPLAY_USEFULLSCREEN, BM_SETCHECK, m_pOptions->GetFullScreen() ? BST_CHECKED : BST_UNCHECKED, 0);

	SendDlgItemMessage(hDlgDisplay, IDC_DISPLAY_STRIPALL, BM_SETCHECK, m_pOptions->GetStripAll() ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlgDisplay, IDC_DISPLAY_STRIPINCOMING, BM_SETCHECK, m_pOptions->GetStripIncoming() ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlgDisplay, IDC_DISPLAY_TIMESTAMP, BM_SETCHECK, m_pOptions->GetShowTimestamp() ? BST_CHECKED : BST_UNCHECKED, 0);

	// Init Ident Dialog
	SendDlgItemMessage(hDlgIdent, IDC_IDENT_USERNAME, EM_LIMITTEXT, 10, 0);
	SetDlgItemText(hDlgIdent, IDC_IDENT_USERNAME, m_pOptions->GetIdentUser().Str());

	SetEditMenuSubclass(GetDlgItem(hDlgIdent, IDC_IDENT_USERNAME), false);

	SendDlgItemMessage(hDlgIdent, IDC_IDENT_PORT, EM_LIMITTEXT, 5, 0);
	SetDlgItemInt(hDlgIdent, IDC_IDENT_PORT, m_pOptions->GetIdentPort(), FALSE);

	SetEditMenuSubclass(GetDlgItem(hDlgIdent, IDC_IDENT_PORT), false);

	SendDlgItemMessage(hDlgIdent, IDC_IDENT_ENABLE, BM_SETCHECK, m_pOptions->GetIdentEnable() ? BST_CHECKED : BST_UNCHECKED, 0);

	// Init Format Dialog

	FormatList& lstFormats = m_pOptions->GetFormatList();

	for(UINT j = 0; j < lstFormats.Count(); ++j)
	{
		Format* pFormat = lstFormats.Item(j);
		_ASSERTE(pFormat != NULL);

		int index = SendDlgItemMessage(hDlgFormat, IDC_FORMAT_EVENT, CB_ADDSTRING, 0, 
			(LPARAM)pFormat->GetEventName().Str());
		_ASSERTE(index != CB_ERR);
		if(index != CB_ERR)
		{
			SendDlgItemMessage(hDlgFormat, IDC_FORMAT_EVENT, CB_SETITEMDATA, index, (LPARAM)pFormat);
		}
	}

	EnumFontFamilies(GetDC(NULL), NULL, (FONTENUMPROC)&OptionDlg::EnumFontFamStub, (LPARAM)this);
	SendDlgItemMessage(hDlgFormat, IDC_FORMAT_FONTSIZE_SPIN, UDM_SETRANGE, 0, MAKELONG(24, 1));
	SendDlgItemMessage(hDlgFormat, IDC_FORMAT_FONTSIZE_SPIN, UDM_SETPOS, 0, MAKELONG(m_pOptions->GetFontSize(), 0));

	SendDlgItemMessage(hDlgFormat, IDC_FORMAT_EVENT, CB_SETCURSEL, 0, 0);
	OnFormatSelEndOk();

	SendDlgItemMessage(hDlgFormat, IDC_FORMAT_STRING, EM_LIMITTEXT, POCKETIRC_MAX_IRC_DISPLAY_LEN, 0);
	SetEditMenuSubclass(GetDlgItem(hDlgFormat, IDC_FORMAT_STRING), true);

	// Init DCC
	switch(m_pOptions->GetLocalAddressMethod())
	{
		default:
			SendDlgItemMessage(hDlgDCC, IDC_DCC_USELOCALIP, BM_SETCHECK, BST_CHECKED, 0);
		break;
		case 1:
			SendDlgItemMessage(hDlgDCC, IDC_DCC_USEREMOTEIP, BM_SETCHECK, BST_CHECKED, 0);
		break;
		case 2:
			SendDlgItemMessage(hDlgDCC, IDC_DCC_USECUSTOMIP, BM_SETCHECK, BST_CHECKED, 0);
		break;
	}

	SendDlgItemMessage(hDlgDCC, IDC_DCC_IGNORECHAT, BM_SETCHECK, m_pOptions->GetIgnoreChat() ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlgDCC, IDC_DCC_IGNORESEND, BM_SETCHECK, m_pOptions->GetIgnoreSend() ? BST_CHECKED : BST_UNCHECKED, 0);

	if(m_pOptions->GetLocalAddress().Size())
		SetDlgItemText(hDlgDCC, IDC_DCC_LOCALIP, m_pOptions->GetLocalAddress().Str());
	if(m_pOptions->GetDetectedAddress().Size())
		SetDlgItemText(hDlgDCC, IDC_DCC_REMOTEIP, m_pOptions->GetDetectedAddress().Str());
	SetDlgItemText(hDlgDCC, IDC_DCC_CUSTOMIP, m_pOptions->GetCustomAddress().Str());

	SetDlgItemInt(hDlgDCC, IDC_DCC_FIRSTPORT, m_pOptions->GetDCCStartPort(), FALSE);
	SetDlgItemInt(hDlgDCC, IDC_DCC_LASTPORT, m_pOptions->GetDCCEndPort(), FALSE);

	// Init Aboot Box
	SetDlgItemText(hDlgAbout, IDC_ABOOT_VERSION, APP_VERSION_STRING);

	SetDlgItemText(hDlgAbout, IDC_ABOOT_REGCODE, m_pOptions->GetRegCode().Str());
	SetEditMenuSubclass(GetDlgItem(hDlgAbout, IDC_ABOOT_REGCODE), false);

	// Gogogo!
	ActivateTab(true);
}

void OptionDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
		case IDOK:
			OnOk();
		break;
		case IDCANCEL:
			EndDialog(m_hwnd, IDCANCEL);
		break;
		case IDC_OPTIONS_REMOVESERVER:
			OnRemoveServer();
		break;
		case IDC_OPTIONS_SERVERNAME:
			switch(HIWORD(wParam))
			{
				case CBN_SELENDOK:
					OnServerNameSelEndOk();
				break;
			}
		break;
		case IDC_DISPLAY_BROWSE:
			OnDisplayBrowse();
		break;
		case IDC_FORMAT_EVENT:
			switch(HIWORD(wParam))
			{
				case CBN_SELENDOK:
					OnFormatSelEndOk();
				break;
			}
		break;
		case IDC_FORMAT_ENABLE:
			OnFormatEnable();
		break;
		case IDC_FORMAT_STRING:
			switch(HIWORD(wParam))
			{
				case EN_CHANGE:
					OnFormatStringChange();
				break;
			}
		break;
		case IDC_FORMAT_RESET:
			OnFormatReset();
		break;
		case IDC_FORMAT_RESETALL:
			OnFormatResetAll();
		break;
		case IDC_FORMAT_TEXTCOLOR:
			OnFormatTextColor();
		break;
		case IDC_FORMAT_BACKCOLOR:
			OnFormatBackColor();
		break;
	}
}

void OptionDlg::OnOk()
{
	USES_CONVERSION;

// Server Settings
	HWND hDlgServer = GetDlg(IDC_OPTIONS_SERVERDLG);

	// Nickname
	TCHAR szNickBuf[POCKETIRC_MAX_NICK_LEN + 1] = _T("");
	int iCopied = GetDlgItemText(hDlgServer, IDC_OPTIONS_NICKNAME, szNickBuf, sizeof(szNickBuf));
	if(iCopied > 0)
	{
		m_pOptions->SetNick(szNickBuf);
	}
	else
	{
		m_pOptions->SetNick(POCKETIRC_DEF_NICK);
	}

	// Real Name
	TCHAR szRealNameBuf[POCKETIRC_MAX_REALNAME_LEN + 1] = _T("");
	GetDlgItemText(hDlgServer, IDC_OPTIONS_REALNAME, szRealNameBuf, sizeof(szRealNameBuf));
	m_pOptions->SetRealName(szRealNameBuf);

	// Server Name
	TCHAR szServerBuf[POCKETIRC_MAX_HOST_NAME_LEN + 1] = _T("");
	iCopied = GetDlgItemText(hDlgServer, IDC_OPTIONS_SERVERNAME, szServerBuf, sizeof(szServerBuf));
	if(iCopied > 0)
	{
		// Server Port
		BOOL bTranslateOk = FALSE;
		UINT uPort = GetDlgItemInt(hDlgServer, IDC_OPTIONS_SERVERPORT, &bTranslateOk, FALSE);
		if(bTranslateOk == FALSE || uPort == 0 || uPort > 65535)
		{	
			uPort = POCKETIRC_DEFAULT_PORT;
		}

		String sPass;
		GetDlgItemString(hDlgServer, IDC_OPTIONS_SERVERPASS, sPass);

		m_pOptions->SetDefaultHost(szServerBuf, uPort, sPass);
	}

	// Quit Message
	TCHAR szQuitMsgBuf[POCKETIRC_MAX_IRC_LINE_LEN + 1] = _T("");
	GetDlgItemText(hDlgServer, IDC_OPTIONS_QUITMSG, szQuitMsgBuf, sizeof(szQuitMsgBuf));
	m_pOptions->SetQuitMsg(szQuitMsgBuf);


// Display Settings
	HWND hDlgDisplay = GetDlg(IDC_OPTIONS_DISPLAYDLG);
	
	m_pOptions->SetFullScreen(SendDlgItemMessage(hDlgDisplay, IDC_DISPLAY_USEFULLSCREEN, BM_GETCHECK, 0, 0) == BST_CHECKED);
	m_pOptions->SetBackAlignTop(SendDlgItemMessage(hDlgDisplay, IDC_DISPLAY_BACKALIGNTOP, BM_GETCHECK, 0, 0) == BST_CHECKED);

	m_pOptions->SetStripAll(SendDlgItemMessage(hDlgDisplay, IDC_DISPLAY_STRIPALL, BM_GETCHECK, 0, 0) == BST_CHECKED);
	m_pOptions->SetStripIncoming(SendDlgItemMessage(hDlgDisplay, IDC_DISPLAY_STRIPINCOMING, BM_GETCHECK, 0, 0) == BST_CHECKED);
	m_pOptions->SetShowTimestamp(SendDlgItemMessage(hDlgDisplay, IDC_DISPLAY_TIMESTAMP, BM_GETCHECK, 0, 0) == BST_CHECKED);

	TCHAR szBackground[MAX_PATH + 1];
	GetDlgItemText(hDlgDisplay, IDC_DISPLAY_BACKGROUND, szBackground, sizeof(szBackground));
	m_pOptions->SetBackImage(szBackground);

// Ident Settings
	HWND hDlgIdent = GetDlg(IDC_OPTIONS_IDENTDLG);

	m_pOptions->SetIdentEnable(SendDlgItemMessage(hDlgIdent, IDC_IDENT_ENABLE, BM_GETCHECK, 0, 0) == BST_CHECKED);
	m_pOptions->SetIdentPort(GetDlgItemInt(hDlgIdent, IDC_IDENT_PORT, NULL, FALSE));

	TCHAR szIdentUser[11];
	iCopied = GetDlgItemText(hDlgIdent, IDC_IDENT_USERNAME, szIdentUser, sizeof(szIdentUser));
	if(iCopied > 0)
	{
		m_pOptions->SetIdentUser(szIdentUser);
	}
	else
	{
		m_pOptions->SetIdentUser(POCKETIRC_DEF_IDENT);
	}

// Format Settings
	HWND hDlgFormat = GetDlg(IDC_OPTIONS_FORMATDLG);

	TCHAR szFontName[LF_FACESIZE];
	GetDlgItemText(hDlgFormat, IDC_FORMAT_FONT, szFontName, LF_FACESIZE);
	m_pOptions->SetFontName(szFontName);

	m_pOptions->SetFontSize(GetDlgItemInt(hDlgFormat, IDC_FORMAT_FONTSIZE, NULL, FALSE));

	// DCC
	HWND hDlgDCC = GetDlg(IDC_OPTIONS_DCCDLG);

	if(SendDlgItemMessage(hDlgDCC, IDC_DCC_USEREMOTEIP, BM_GETCHECK, 0, 0) == BST_CHECKED)
		m_pOptions->SetLocalAddressMethod(1);
	else if(SendDlgItemMessage(hDlgDCC, IDC_DCC_USECUSTOMIP, BM_GETCHECK, 0, 0) == BST_CHECKED)
		m_pOptions->SetLocalAddressMethod(2);
	else
		m_pOptions->SetLocalAddressMethod(0);

	TCHAR szAddr[20] = _T("");
	GetDlgItemText(hDlgDCC, IDC_DCC_CUSTOMIP, szAddr, sizeof(szAddr)/sizeof(szAddr[0]));

	if(inet_addr(T2CA(szAddr)) != INADDR_NONE)
	{
		m_pOptions->SetCustomAddress(szAddr);
	}
	else
	{
		if(m_pOptions->GetLocalAddressMethod() == 2)
		{
			m_pOptions->SetLocalAddressMethod(0);
		}
	}

	m_pOptions->SetDCCStartPort((USHORT)GetDlgItemInt(hDlgDCC, IDC_DCC_FIRSTPORT, NULL, FALSE));
	m_pOptions->SetDCCEndPort((USHORT)GetDlgItemInt(hDlgDCC, IDC_DCC_LASTPORT, NULL, FALSE));

	m_pOptions->SetIgnoreChat(SendDlgItemMessage(hDlgDCC, IDC_DCC_IGNORECHAT, BM_GETCHECK, 0, 0) == BST_CHECKED);
	m_pOptions->SetIgnoreSend(SendDlgItemMessage(hDlgDCC, IDC_DCC_IGNORESEND, BM_GETCHECK, 0, 0) == BST_CHECKED);

// Aboot Box
	HWND hDlgAboot = GetDlg(IDC_OPTIONS_ABOUTDLG);

	TCHAR szRegCode[100] = _T("");
	GetDlgItemText(hDlgAboot, IDC_ABOOT_REGCODE, szRegCode, sizeof(szRegCode)/sizeof(TCHAR));
	m_pOptions->SetRegCode(szRegCode);

	EndDialog(m_hwnd, IDOK);
}

void OptionDlg::OnRemoveServer()
{
	HWND hDlgServer = GetDlg(IDC_OPTIONS_SERVERDLG);

	int index = SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, CB_GETCURSEL, 0, 0);
	if(index != CB_ERR)
	{
		Host* pHost = (Host*)SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, CB_GETITEMDATA, index, 0);
		_ASSERTE(pHost != NULL);

		SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, CB_DELETESTRING, index, 0);

		if(pHost != NULL)
		{
			HostList& lstHosts = m_pOptions->GetHostList();

			lstHosts.RemoveHost(pHost);
		}

		SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, CB_SETCURSEL, 0, 0);
		OnServerNameSelEndOk();
	}
}

void OptionDlg::OnServerNameSelEndOk()
{
	HWND hDlgServer = GetDlg(IDC_OPTIONS_SERVERDLG);

	int index = SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, CB_GETCURSEL, 0, 0);
	if(index != CB_ERR)
	{
		Host* pHost = (Host*)SendDlgItemMessage(hDlgServer, IDC_OPTIONS_SERVERNAME, CB_GETITEMDATA, index, 0);
		_ASSERTE(pHost != NULL);
		if(pHost != NULL)
		{
			SetDlgItemInt(hDlgServer, IDC_OPTIONS_SERVERPORT, pHost->GetPort(), FALSE);
			SetDlgItemText(hDlgServer, IDC_OPTIONS_SERVERPASS, pHost->GetPass().Str());
		}
	}
}

void OptionDlg::OnFormatSelEndOk()
{
	HWND hDlgFormat = GetDlg(IDC_OPTIONS_FORMATDLG);

	Format* pFormat = (Format*)GetCurFormatSel();
	_ASSERTE(pFormat != NULL);
	if(pFormat != NULL)
	{
		SendDlgItemMessage(hDlgFormat, IDC_FORMAT_ENABLE, BM_SETCHECK, pFormat->IsEnabled() ? BST_CHECKED : BST_UNCHECKED, 0);
		SetDlgItemText(hDlgFormat, IDC_FORMAT_STRING, pFormat->GetFormat().Str());
	}

	InvalidateRect(GetDlgItem(hDlgFormat, IDC_FORMAT_EXAMPLE), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlgFormat, IDC_FORMAT_EXAMPLE));
}

void OptionDlg::OnFormatEnable()
{
	HWND hDlgFormat = GetDlg(IDC_OPTIONS_FORMATDLG);

	Format* pFormat = (Format*)GetCurFormatSel();
	_ASSERTE(pFormat != NULL);
	if(pFormat != NULL)
	{
		pFormat->SetEnabled(SendDlgItemMessage(hDlgFormat, IDC_FORMAT_ENABLE, BM_GETCHECK, 0, 0) == BST_CHECKED);
	}
}

void OptionDlg::OnFormatStringChange()
{
	HWND hDlgFormat = GetDlg(IDC_OPTIONS_FORMATDLG);

	Format* pFormat = (Format*)GetCurFormatSel();
	_ASSERTE(pFormat != NULL);
	if(pFormat != NULL)
	{
		TCHAR buf[POCKETIRC_MAX_IRC_DISPLAY_LEN + 1] = _T("");
		GetDlgItemText(hDlgFormat, IDC_FORMAT_STRING, buf, sizeof(buf));

		pFormat->SetFormat(buf);
	}

	InvalidateRect(GetDlgItem(hDlgFormat, IDC_FORMAT_EXAMPLE), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlgFormat, IDC_FORMAT_EXAMPLE));
}

void OptionDlg::OnFormatReset()
{
	Format* pFormat = (Format*)GetCurFormatSel();
	_ASSERTE(pFormat != NULL);
	if(pFormat != NULL)
	{
		pFormat->SetEnabled(Options::GetDefaultEnable(pFormat->GetEventID(), pFormat->IsIncoming()));
		pFormat->SetFormat(Options::GetDefaultFormat(pFormat->GetEventID(), pFormat->IsIncoming()));
	}
	OnFormatSelEndOk();
}

void OptionDlg::OnFormatResetAll()
{
	FormatList& lstFormats = m_pOptions->GetFormatList();

	for(UINT j = 0; j < lstFormats.Count(); ++j)
	{
		Format* pFormat = lstFormats.Item(j);
		_ASSERTE(pFormat != NULL);

		pFormat->SetEnabled(Options::GetDefaultEnable(pFormat->GetEventID(), pFormat->IsIncoming()));
		pFormat->SetFormat(Options::GetDefaultFormat(pFormat->GetEventID(), pFormat->IsIncoming()));
	}
	OnFormatSelEndOk();
}

void OptionDlg::OnFormatTextColor()
{
	HWND hDlgFormat = GetDlg(IDC_OPTIONS_FORMATDLG);

	COLORREF crCustom[16];

	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(cc));

	cc.lStructSize = sizeof(cc);
	cc.Flags = CC_ANYCOLOR | CC_RGBINIT;
	cc.hwndOwner = m_hwnd;
	cc.lpCustColors = crCustom;

	cc.rgbResult = m_pOptions->GetTextColor();

	if(ChooseColour(&cc))
	{
		m_pOptions->SetTextColor(cc.rgbResult);

		InvalidateRect(GetDlgItem(hDlgFormat, IDC_FORMAT_TEXTCOLOR), NULL, FALSE);
		UpdateWindow(GetDlgItem(hDlgFormat, IDC_FORMAT_TEXTCOLOR));
	}
}

void OptionDlg::OnFormatBackColor()
{
	HWND hDlgFormat = GetDlg(IDC_OPTIONS_FORMATDLG);

	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(cc));

	cc.lStructSize = sizeof(cc);
	cc.Flags = CC_ANYCOLOR | CC_RGBINIT;
	cc.hwndOwner = m_hwnd;

	cc.rgbResult = m_pOptions->GetBackColor();

	if(ChooseColour(&cc))
	{
		m_pOptions->SetBackColor(cc.rgbResult);

		InvalidateRect(GetDlgItem(hDlgFormat, IDC_FORMAT_BACKCOLOR), NULL, FALSE);
		UpdateWindow(GetDlgItem(hDlgFormat, IDC_FORMAT_BACKCOLOR));
	}
}

void OptionDlg::OnDisplayBrowse()
{
	HWND hDlgDisplay = GetDlg(IDC_OPTIONS_DISPLAYDLG);

	TCHAR szFileName[MAX_PATH + 1] = _T("");

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hwnd;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = szFileName;
	ofn.lpstrTitle = _T("Choose Background");
	ofn.lpstrFilter = _T("Images(*.bmp,*.gif,*.png,*.jpg)\0*.bmp;*.gif;*.png;*.jpg\0All Files(*.*)\0*.*\0");
	ofn.Flags = OFN_EXPLORER;

	if(GetOpenFileName(&ofn))
	{
		SetDlgItemText(hDlgDisplay, IDC_DISPLAY_BACKGROUND, ofn.lpstrFile);
	}
}

LRESULT OptionDlg::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* lpnmh = (NMHDR*)lParam;
	switch(lpnmh->idFrom)
	{
	case IDC_OPTIONS_TAB:
		switch(lpnmh->code)
		{
		case TCN_SELCHANGE:
			ActivateTab(true);
			break;
		case TCN_SELCHANGING:
			ActivateTab(false);
			break;
		}
		break;
	}
	return FALSE;
}

void OptionDlg::OnSize(WPARAM wParam, LPARAM lParam)
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_OPTIONS_TAB);
	TabCtrl_FitToParent(hTab);

	ActivateTab(true);
}

void OptionDlg::OnDrawItem(WPARAM wParam, LPARAM lParam)
{
	DRAWITEMSTRUCT* pdi = (DRAWITEMSTRUCT*)lParam;
	switch(wParam)
	{
		case IDC_FORMAT_EXAMPLE:
		{
			Formatter formatter;
			StringFormat fmt;

			fmt.bg = -1;
			fmt.fg = m_pOptions->GetTextColor();
			fmt.bold = false;
			fmt.underline = false;
			fmt.inverse = false;

			formatter.SetDefaultFmt(fmt);

			Format* pFormat = GetCurFormatSel();
			_ASSERTE(pFormat != NULL);

			SetBkColor(pdi->hDC, m_pOptions->GetBackColor());
			ExtTextOut(pdi->hDC, 0, 0, ETO_OPAQUE, &pdi->rcItem, _T(""), 0, NULL);

			if(pFormat != NULL)
			{
				String sFmt = pFormat->GetFormat();

				NetworkEvent event(IRC_CMD_UNKNOWN, 10, &String(_T("ZERO")), &String(_T("ONE")), &String(_T("TWO")), &String(_T("THREE")), &String(_T("FOUR")), &String(_T("FIVE")), &String(_T("SIX")), &String(_T("SEVEN")), &String(_T("EIGHT")), &String(_T("NINE")));
				event.SetPrefix(_T("NICK!~IDENT@HOST"));

				String sMsg = EventFormat(event, _T("YOU"), sFmt);

				formatter.FormatOut(pdi->hDC, sMsg, pdi->rcItem, Formatter::FMT_WRAP);
			}
		}
		break;
		case IDC_FORMAT_TEXTCOLOR:
			SetBkColor(pdi->hDC, m_pOptions->GetTextColor());
			ExtTextOut(pdi->hDC, 0, 0, ETO_OPAQUE, &pdi->rcItem, _T(""), 0, NULL);
		break;
		case IDC_FORMAT_BACKCOLOR:
			SetBkColor(pdi->hDC, m_pOptions->GetBackColor());
			ExtTextOut(pdi->hDC, 0, 0, ETO_OPAQUE, &pdi->rcItem, _T(""), 0, NULL);
		break;
	}
}

void OptionDlg::OnVScroll(WPARAM wParam, LPARAM lParam)
{
	HWND hScroll = (HWND)lParam;
	if(IsWindow(hScroll))
	{
		int nScrollCode = LOWORD(wParam);
		int nPos = HIWORD(wParam);

		SCROLLINFO si = {sizeof(SCROLLINFO), SIF_ALL};
		GetScrollInfo(hScroll, SB_CTL, &si);

		switch (nScrollCode)
		{
			case SB_THUMBPOSITION:
				si.nPos = si.nTrackPos + si.nMin;
			break;
			case SB_THUMBTRACK:
				si.nPos = si.nTrackPos;
			break;
			case SB_LINEDOWN:
				si.nPos++;
			break;
			case SB_LINEUP:
				si.nPos--;
			break;
			case SB_PAGEDOWN:
				si.nPos += si.nPage;
			break;
			case SB_PAGEUP:
				si.nPos -= si.nPage;
			break;
			case SB_BOTTOM:
				si.nPos = si.nMax;
			break;
			case SB_TOP:
				si.nPos = si.nMin;
			break;
		}

		si.fMask = SIF_POS;
		SetScrollInfo(hScroll, SB_CTL, &si, TRUE);
		GetScrollInfo(hScroll, SB_CTL, &si);

		HWND hTab = GetDlgItem(m_hwnd, IDC_OPTIONS_TAB);
		_ASSERTE(IsWindow(hTab));

		int index = TabCtrl_GetCurSel(hTab);
		if(index != -1)
		{
			TCITEM tci = {TCIF_PARAM};
			bool bGetItemOk = TabCtrl_GetItem(hTab, index, &tci) != 0;
			_ASSERTE(bGetItemOk != FALSE);

			HWND hDlg = (HWND)tci.lParam;
			_ASSERTE(IsWindow(hDlg));

			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0 - si.nPos, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
		}
	}
}

void OptionDlg::OnSettingChange(WPARAM wParam, LPARAM lParam)
{
	if(wParam == SPI_SETSIPINFO)
	{
		UpdateSize();
	}
}

void OptionDlg::UpdateSize()
{
	SIPINFO si = {sizeof(SIPINFO)};
	SipGetInfo(&si);

	if(!(si.fdwFlags & SIPF_ON))
	{
		HWND hMenuBar = SHFindMenuBar(m_hwnd);
		_ASSERTE(IsWindow(hMenuBar));

		RECT rcMenuBar = {0};
		GetWindowRect(hMenuBar, &rcMenuBar);
		si.rcVisibleDesktop.bottom -= rcMenuBar.bottom - rcMenuBar.top;
	}

	MoveWindow(m_hwnd, si.rcVisibleDesktop.left, si.rcVisibleDesktop.top,
		si.rcVisibleDesktop.right - si.rcVisibleDesktop.left,
		si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top,
		FALSE);
}
