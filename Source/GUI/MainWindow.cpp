#include "PocketIRC.h"
#include "MainWindow.h"

#include "IrcString.h"
#include "Config\Options.h"
#include "OptionDlg.h"
#include "ChannelsDlg.h"

#include "StatusWindow.h"
#include "ChannelWindow.h"
#include "QueryWindow.h"
#include "EventFormat.h"

#include "resource.h"

enum PI_TIMERID {
	PI_TIMERID_IDLEPING = 1
};

const unsigned PI_PING_TIMERFREQ = 10*1000; // Frequency to check idle time
const unsigned PI_PING_TIMEOUT = 30*1000; // How much idle time before sending PING to server

/////////////////////////////////////////////////////////////////////////////
//	Static Methods
/////////////////////////////////////////////////////////////////////////////

bool MainWindow::ActivatePrevious()
{
	_TRACE("MainWindow::ActivatePrevious()");

	HWND hwnd = FindWindow(ClassName(), APP_NAME);
	if(IsWindow(hwnd))
	{
		ShowWindow(hwnd, SW_SHOWNORMAL);
		SetForegroundWindow(hwnd);
		return true;
	}
	return false;		
}

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
{
	_TRACE("MainWindow(0x%08X)::MainWindow()", this);
	
	ZeroMemory(&m_shackInfo, sizeof(m_shackInfo));
	m_shackInfo.cbSize = sizeof(m_shackInfo);

	m_hMenuBar = NULL;
	m_hServerMenu = NULL;

	m_pSession = NULL;
	m_pTransport = NULL;
	m_pIdent = NULL;
	m_hTabMenu = NULL;

	m_bConnected = false;

	ZeroMemory(&m_rcOldClient, sizeof(RECT));
}

MainWindow::~MainWindow()
{
	_TRACE("MainWindow(0x%08X)::~MainWindow()", this);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Creation
/////////////////////////////////////////////////////////////////////////////

HRESULT MainWindow::Init()
{
	_TRACE("MainWindow(0x%08X)::Init()", this);

	HRESULT hr = g_Options.Load(POCKETIRC_REG_HKEY_ROOT, POCKETIRC_REG_KEY_NAME);
	_ASSERTE(SUCCEEDED(hr));

	if(FAILED(hr))
	{
		MessageBoxFmt(NULL, _T("There was an error(0x%08X) loading %s settings from the registry."), 
			APP_NAME, MB_OK | MB_ICONEXCLAMATION, hr, APP_NAME);
	}

	Options::UpdateResourceCache();

	hr = TabStrip::RegisterClass();
	_ASSERTE(SUCCEEDED(hr));

	return hr;
}

HRESULT MainWindow::UnInit()
{
	_TRACE("MainWindow(0x%08X)::UnInit()", this);

	HRESULT hr = g_Options.Save(POCKETIRC_REG_HKEY_ROOT, POCKETIRC_REG_KEY_NAME);
	_ASSERTE(SUCCEEDED(hr));

	if(FAILED(hr))
	{
		MessageBoxFmt(NULL, _T("There was an error(0x%08X) saving %s settings to the registry."), 
			APP_NAME, MB_OK | MB_ICONEXCLAMATION, hr, APP_NAME);
	}

	hr = TabStrip::UnRegisterClass();
	_ASSERTE(SUCCEEDED(hr));

	return hr;
}

HRESULT MainWindow::Create()
{
	_TRACE("MainWindow(0x%08X)::Create()", this);
	
	HRESULT hr = Window::Create(NULL, APP_NAME, WS_VISIBLE, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT);
	
	if(SUCCEEDED(hr))
	{
		StatusWindow* pWindow = new StatusWindow;
		pWindow->SetMainWindow(this);
		pWindow->SetTitle(_T("Status"));
		pWindow->Create();

		SetDefaultWindow(pWindow);

		pWindow->Print(_T("Welcome to ") APP_NAME _T(" ") APP_VERSION_STRING);
		pWindow->Print(_T("Tap Help on your Start menu for tips on getting started."));

		UpdateSize();

		ShowWindow(m_hwnd, SW_SHOWNORMAL);
		UpdateWindow(m_hwnd);
	}
		
	return hr;
}

void MainWindow::SetSession(Session* pSession)
{
	_TRACE("MainWindow(0x%08X)::SetSession(0x%08X)", this, pSession);
	m_pSession = pSession;
}

void MainWindow::SetTransport(ITransport* pTransport)
{
	_TRACE("MainWindow(0x%08X)::SetTransport(0x%08X)", this, pTransport);
	m_pTransport = pTransport;
}

void MainWindow::SetIdentServer(IIdentServer* pIdent)
{
	_TRACE("MainWindow(0x%08X)::SetIdentServer(0x%08X)", this, pIdent);
	m_pIdent = pIdent;
}

void MainWindow::SetDCCHandler(IDCCHandler* pDCCHandler)
{
	_TRACE("MainWindow(0x%08X)::SetDCCHandler(0x%08X)", this, pDCCHandler);
	m_pDCCHandler = pDCCHandler;
}

void MainWindow::UpdateSize()
{
	SHFullScreen(m_hwnd, g_Options.GetFullScreen() ? SHFS_HIDETASKBAR : SHFS_SHOWTASKBAR);

	SIPINFO si = {sizeof(SIPINFO)};
	SipGetInfo(&si);

	if(g_Options.GetFullScreen())
	{
		si.rcVisibleDesktop.top = 0;
	}

	if(!(si.fdwFlags & SIPF_ON))
	{
		RECT rcMenuBar;
		GetWindowRect(m_hMenuBar, &rcMenuBar);
		si.rcVisibleDesktop.bottom -= rcMenuBar.bottom - rcMenuBar.top;
	}

	MoveWindow(m_hwnd, si.rcVisibleDesktop.left, si.rcVisibleDesktop.top,
		si.rcVisibleDesktop.right - si.rcVisibleDesktop.left,
		si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top,
		FALSE);
}

void MainWindow::OptionsChanged()
{
	SetWindowPos(m_hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);

	UpdateSize();

	for(UINT i = 0; i < m_vecChildren.Size(); ++i)
	{
		m_vecChildren[i]->OnOptionsChanged();
	}
}

void MainWindow::SetCurrentTarget(const String& sTarget) 
{ 
	_TRACE("MainWindow(0x%08X)::SetCurrentTarget(\"%s\")", this, sTarget.Str());
	m_sCurrentTarget = sTarget; 
}

bool MainWindow::ProcessSlashCommand()
{
	_TRACE("MainWindow(0x%08X)::ProcessSlashCommand()", this);
	typedef struct _SLASHMAP
	{
		TCHAR* name;
		void (MainWindow::*cmd)(const String& s);
	} SLASHMAP;

	SLASHMAP slashMap[] = 
	{
		// irc commands
		{ _T("quit"),   &MainWindow::SlashQuit },
		{ _T("part"),   &MainWindow::SlashPart },
		{ _T("say"),    &MainWindow::SlashSay },
		{ _T("msg"),    &MainWindow::SlashMsg },
		{ _T("query"),  &MainWindow::SlashQuery },
		{ _T("close"),  &MainWindow::SlashClose },
		{ _T("me"),     &MainWindow::SlashMe },
		{ _T("kick"),   &MainWindow::SlashKick },
		{ _T("invite"), &MainWindow::SlashInvite },
		{ _T("notice"), &MainWindow::SlashNotice },
		{ _T("topic"),  &MainWindow::SlashTopic },
		{ _T("raw"),    &MainWindow::SlashRaw },
		{ _T("away"),   &MainWindow::SlashAway },
		{ _T("ping"),   &MainWindow::SlashPing },
		{ _T("ctcp"),   &MainWindow::SlashCTCP },

		// application commands
		{ _T("exit"),   &MainWindow::SlashExit },
		{ _T("connect"),&MainWindow::SlashConnect },
		{ _T("disconnect"),&MainWindow::SlashDisconnect },
		{ _T("server"), &MainWindow::SlashServer },
	};

	String sInput = GetInput();
	if(sInput.Size() > 0 && sInput[0] == _T('/'))
	{
		String sCmd = sInput.GetWord(0).SubStr(1);

		for(int i = 0; i < sizeof(slashMap)/sizeof(*slashMap); ++i)
		{
			if(sCmd.Compare(slashMap[i].name, false))
			{
				String params = sInput.GetWord(1, true);
				(this->*slashMap[i].cmd)(params);
				ClearInput();
				return true;
			}
		}

		SlashRaw(sInput.SubStr(1));
		ClearInput();
		return true;
	}
	return false;
}

void MainWindow::SlashPart(const String& s)
{
	String sChannel = s.GetWord(0);
	String sMsg = s.GetWord(1, true);

	if(!sChannel.Size())
	{
		sChannel = GetCurrentTarget();
	}

	m_pSession->Part(sChannel, sMsg);
}

void MainWindow::SlashRaw(const String& s)
{
	m_pSession->Raw(s);
}

void MainWindow::SlashQuit(const String& s)
{
	String msg = s;
	if(msg.Size() == 0)
	{
		msg = g_Options.GetQuitMsg();
	}
	m_pSession->Quit(msg);
}

void MainWindow::SlashMsg(const String& s)
{
	String sTo = s.GetWord(0);
	String sMsg = s.GetWord(1, true);
	m_pSession->PrivMsg(sTo, sMsg);
}

void MainWindow::SlashQuery(const String& s)
{
	String sTo = s.GetWord(0);
	OpenQuery(sTo);
}

void MainWindow::SlashClose(const String& s)
{
	String sQuery = GetCurrentTarget();
	CloseQuery(sQuery);
}

void MainWindow::SlashSay(const String& s)
{
	String sTo = GetCurrentTarget();
	m_pSession->PrivMsg(sTo, s);
}

void MainWindow::SlashMe(const String& s)
{
	String sTo = GetCurrentTarget();
	m_pSession->Action(sTo, s);
}

void MainWindow::SlashKick(const String& s)
{
	String sTarget = s.GetWord(0);
	String sUser = s.GetWord(1);
	String sReason = s.GetWord(2, true);
	m_pSession->Kick(sTarget, sUser, sReason);
}

void MainWindow::SlashInvite(const String& s)
{
	String sUser = s.GetWord(0);
	String sTarget = s.GetWord(1);
	if(!sTarget.Size())
	{
		sTarget = GetCurrentTarget();
	}
	m_pSession->Invite(sUser, sTarget);
}

void MainWindow::SlashNotice(const String& s)
{
	String sTo = s.GetWord(0);
	String sMsg = s.GetWord(1, true);
	m_pSession->Notice(sTo, sMsg);
}


void MainWindow::SlashTopic(const String& s)
{
	String sTarget = s.GetWord(0);
	String sTopic = s.GetWord(1, true);
	m_pSession->Topic(sTarget, sTopic);
}

void MainWindow::SlashExit(const String& s)
{
	PostMessage(m_hwnd, WM_CLOSE, 0, 0);
}

void MainWindow::SlashAway(const String& s)
{
	String sMsg = s.GetWord(0, true);
	m_pSession->Away(sMsg);
}

void MainWindow::SlashPing(const String& s)
{
	String sTarget = s.GetWord(0);
	m_pSession->CTCPPing(sTarget);
}

void MainWindow::SlashCTCP(const String& s)
{
	String sTarget = s.GetWord(0);
	String sCmd = s.GetWord(1);
	String sMsg = s.GetWord(2, true);
	m_pSession->CTCP(sTarget, sCmd, sMsg);
}

void MainWindow::SlashConnect(const String& s)
{
	Connect();
}

void MainWindow::SlashDisconnect(const String& s)
{
	Disconnect();
}

void MainWindow::SlashServer(const String& s)
{
	String sHost = s.GetWord(0);
	String sPass = s.GetWord(1);

	USHORT uPort = POCKETIRC_DEFAULT_PORT;
	String sServer = sHost;

	TCHAR* port = _tcschr(sHost.Str(), ':');
	if(port)
	{
		uPort = (USHORT)_tcstoul(port + 1, NULL, 10);
		sServer = sHost.SubStr(0, port - sHost.Str() - 1);
	}

	g_Options.SetDefaultHost(sServer, uPort, sPass);

	if(m_bConnected)
	{
		Disconnect();
	}
	Connect();
}

/////////////////////////////////////////////////////////////////////////////
//	IConnectStateNotify
/////////////////////////////////////////////////////////////////////////////

void MainWindow::OnConnectStateChange(const NetworkEvent &e)
{
	bool connected = (e.GetEventID() == SYS_EVENT_CONNECTED);
	_TRACE("MainWindow(0x%08X)::OnConnectStateChange(connected == %s)", this, connected ? _T("true") : _T("false"));

	if(m_bConnected != connected)
	{
		m_bConnected = connected;

		Host* pHost = g_Options.GetHostList().GetDefault();
		_ASSERTE(pHost != NULL);

		DispatchToChannels(e);

		if(m_bConnected)
		{
			const String &sPass = pHost->GetPass();
			if(sPass.Size())
			{
				m_pSession->Pass(sPass);
			}

			m_pSession->User(g_Options.GetIdentUser(), g_Options.GetRealName());
			m_pSession->Nick(g_Options.GetNick());

			g_Options.SetLocalAddress(m_pTransport->GetLocalAddress());
		}
		else
		{
			_ASSERTE(m_pIdent);
			m_pIdent->Stop();
		}
	}
}

void MainWindow::OnJoin(const NetworkEvent& event)
{
	_TRACE("MainWindow(0x%08X)::OnJoin()", this);

	String sUser = GetPrefixNick(event.GetPrefix());
	const String& sChannel = event.GetParam(0);

	if(m_pSession->IsMe(sUser))
	{
		IDisplayWindow* pWindow = GetDisplayWindow(sChannel);
		if(!pWindow)
		{
			CreateChannelWindow(sChannel);
		}
	}
}

void MainWindow::OnPart(const NetworkEvent& event)
{
	_TRACE("MainWindow(0x%08X)::OnPart()", this);

	String sUser = GetPrefixNick(event.GetPrefix());
	const String& sChannel = event.GetParam(0);
}

void MainWindow::OnKick(const NetworkEvent& event)
{
	_TRACE("MainWindow(0x%08X)::OnKick()", this);
	const String& sChannel = event.GetParam(0);
	const String& sUser = event.GetParam(1);
}

void MainWindow::OnNick(const NetworkEvent& event)
{
	_TRACE("MainWindow(0x%08X)::OnNick()", this);
	String sUser = GetPrefixNick(event.GetPrefix());
	const String& sNewNick = event.GetParam(0);

	//If there is an open query to this user, change it over to the new nick
	IDisplayWindow* pQuery = GetDisplayWindow(sUser);
	if(pQuery)
	{
		// But don't change it over if there is already a query to the new nick!
		// otherwise we'd have duplicate queries to one person and that would break
		// things.  Having a query to a user that has left isn't a problem.

		IDisplayWindow* pExistingQuery = GetDisplayWindow(sNewNick);
		if(pExistingQuery == NULL)
		{
			pQuery->SetKey(sNewNick);
			pQuery->SetTitle(sNewNick);
		}
	}

	DispatchToChannelsWithUser(event, sUser);
}

void MainWindow::OnQuit(const NetworkEvent& event)
{
	_TRACE("MainWindow(0x%08X)::OnQuit()", this);

	String sUser = GetPrefixNick(event.GetPrefix());

	if(m_pSession->IsMe(sUser))
	{
		// Don't think we ever get QUIT for ourself
	}
	else
	{
		DispatchToChannelsWithUser(event, sUser);
	}
}

void MainWindow::OnPrivMsg(const NetworkEvent& event)
{
	//_TRACE("MainWindow(0x%08X)::OnPrivMsg()", this);

	String sUser = GetPrefixNick(event.GetPrefix());
	const String& sTarget = event.GetParam(0);

	if(m_pSession->IsMe(sTarget))
	{
		OpenQuery(sUser);
	}
}

void MainWindow::OnRplWelcome(const NetworkEvent& event)
{
	_TRACE("MainWindow(0x%08X)::OnRplWelcome()", this);

	const String& sMe = event.GetParam(0);
	_ASSERTE(sMe.Size());

	m_pSession->UserHost(sMe);

	FavouriteChannelList& chanList = g_Options.GetFavouriteChannelList();

	UINT count = chanList.Count();
	for(UINT i = 0; i < count; ++i)
	{
		FavouriteChannel* chan = chanList.GetChannel(i);
		if(chan->AutoJoin)
		{
			m_pSession->Join(chan->Name, chan->Key);
		}
	}
}

void MainWindow::OnRplUserHost(const NetworkEvent& event)
{
	_TRACE("MainWindow(0x%08X)::OnRplUserHost()", this);

	String sHost = event.GetParam(1);

	TCHAR* user = _tcschr(sHost.Str(), '=');
	if(user && user > sHost.Str())
	{
		String sUser = sHost.SubStr(0, user - sHost.Str());
		if(m_pSession->IsMe(sUser))
		{
			TCHAR* ip = _tcschr(sHost.Str(), '@');
			if(ip)
			{
				USES_CONVERSION;

				ip++;

				IN_ADDR addr;
				addr.S_un.S_addr = inet_addr(T2CA(ip));
				if(addr.S_un.S_addr != INADDR_NONE)
				{
					char* newip = inet_ntoa(addr);
					if(newip)
					{
						// Gah.  Blah.  Bleh.
						g_Options.SetDetectedAddress(A2CT(newip));
					}
				}
			}
		}
	}
}

void MainWindow::OnEvent(const NetworkEvent &networkEvent)
{
	//_TRACE("MainWindow(0x%08X)::OnEvent(\"%s\")", this, networkEvent.GetEvent().Str());

	#define HANDLE_IRC_EVENT(id, h) case id: h(networkEvent); break;

	if(networkEvent.IsIncoming())
	{
		switch(networkEvent.GetEventID())
		{
			HANDLE_IRC_EVENT(IRC_CMD_NICK, OnNick)
			HANDLE_IRC_EVENT(IRC_CMD_JOIN, OnJoin)
			HANDLE_IRC_EVENT(IRC_CMD_PART, OnPart)
			HANDLE_IRC_EVENT(IRC_CMD_KICK, OnKick)
			HANDLE_IRC_EVENT(IRC_CMD_QUIT, OnQuit)
			HANDLE_IRC_EVENT(IRC_CMD_PRIVMSG, OnPrivMsg)
			HANDLE_IRC_EVENT(IRC_CTCP_ACTION, OnPrivMsg)
			HANDLE_IRC_EVENT(IRC_RPL_WELCOME, OnRplWelcome)
			HANDLE_IRC_EVENT(IRC_RPL_USERHOST, OnRplUserHost)
		}
	}

	switch(networkEvent.GetEventID())
	{
		HANDLE_IRC_EVENT(SYS_EVENT_CONNECTED, OnConnectStateChange)
		HANDLE_IRC_EVENT(SYS_EVENT_CLOSE, OnConnectStateChange)
	}

	DispatchEvent(networkEvent);
}

/////////////////////////////////////////////////////////////////////////////
//	MainWindow
/////////////////////////////////////////////////////////////////////////////

void MainWindow::AddWindowTab(ITabWindow* pWindow)
{
	_TRACE("MainWindow(0x%08X)::AddWindowTab(0x%08X)", this, pWindow);

	m_TabStrip.AddTab(pWindow->GetTabTitle(), pWindow->GetTabWindow(), (LPARAM)pWindow);
	m_vecChildren.Append(pWindow);

	// Force resize to fit new window
	ZeroMemory(&m_rcOldClient, sizeof(RECT));
	this->OnSize(0, 0);
}

void MainWindow::SetWindowTabTitle(ITabWindow* pWindow, const String& sTitle)
{
	_TRACE("MainWindow(0x%08X)::SetWindowTabTitle(0x%08X, \"%s\")", this, pWindow, sTitle.Str());

	UINT index;
	if(m_TabStrip.FindTab((LPARAM)pWindow, &index))
	{
		m_TabStrip.SetTabText(index, sTitle, true);
	}
}

void MainWindow::SetWindowTabHighlight(ITabWindow* pWindow, int iHighlight)
{
	_TRACE("MainWindow(0x%08X)::SetWindowTabColor(0x%08X, %d)", this, pWindow, iHighlight);

	UINT index;
	if(m_TabStrip.FindTab((LPARAM)pWindow, &index))
	{
		m_TabStrip.SetTabColor(index, g_Options.GetHighlight(iHighlight), true);
	}
}

void MainWindow::SetWindowTabMenu(HMENU hMenu)
{
	_TRACE("MainWindow(0x%08X)::SetWindowTabMenu(0x%08X)", this, hMenu);

	m_hTabMenu = hMenu;
}

void MainWindow::RemoveWindowTab(ITabWindow* pWindow)
{
	_TRACE("MainWindow(0x%08X)::RemoveWindowTab(0x%08X)", this, pWindow);

	UINT index;
	if(m_TabStrip.FindTab((LPARAM)pWindow, &index))
	{
		m_TabStrip.RemoveTab(index);
	}

	index = m_vecChildren.Find(pWindow);
	if(index != Vector<ITabWindow*>::NPOS)
	{
		m_vecChildren.Erase(index);
	}
}

String MainWindow::GetInput()
{
	_TRACE("MainWindow(0x%08X)::GetInput()", this);

	HWND hCombo = m_InputBar.GetWindow();
	_ASSERTE(IsWindow(hCombo));

	TCHAR buf[POCKETIRC_MAX_SAY_LEN + 1] = _T("");
	int iCopied = GetWindowText(hCombo, buf, sizeof(buf));			
	if(iCopied > 0)
	{
		int index = SendMessage(hCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)buf);
		if(index != CB_ERR)
		{
			SendMessage(hCombo, CB_DELETESTRING, index, 0);
		}
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
	}
	return String(buf);
}

void MainWindow::ClearInput()
{
	_TRACE("MainWindow(0x%08X)::ClearInput()", this);

	HWND hCombo = m_InputBar.GetWindow();
	_ASSERTE(IsWindow(hCombo));

	SetWindowText(hCombo, _T(""));
}

void MainWindow::InsertInput(const String& str)
{
	_TRACE("MainWindow(0x%08X)::InsertInput(\"%s\")", this, str.Str());
	SendMessage(::GetWindow(m_InputBar.GetWindow(), GW_CHILD), EM_REPLACESEL, 0, (LPARAM)str.Str());
}

void MainWindow::OpenQuery(const String& sUser)
{
	_TRACE("MainWindow(0x%08X)::OpenQuery(\"%s\")", this, sUser.Str());

	if(!IsChannel(sUser))
	{
		IDisplayWindow* pQuery = GetDisplayWindow(sUser);
		if(!pQuery)
		{
			CreateQueryWindow(sUser);
		}
	}
}

void MainWindow::CloseQuery(const String& sUser)
{
	_TRACE("MainWindow(0x%08X)::CloseQuery(\"%s\")", this, sUser.Str());

	IDisplayWindow* pQuery = GetDisplayWindow(sUser);
	if(pQuery)
	{
		pQuery->Close();
	}
}

IDisplayWindow* MainWindow::CreateChannelWindow(const String& sChannel)
{
	_TRACE("MainWindow(0x%08X)::CreateChannelWindow(\"%s\")", this, sChannel.Str());

	Channel* pChannel = m_pSession->GetChannel(sChannel);
	_ASSERTE(pChannel);
	if(pChannel)
	{
		ChannelWindow* pWindow = new ChannelWindow;
		pWindow->SetMainWindow(this);
		pWindow->SetChannel(sChannel);

		pWindow->Create();

		return pWindow;
	}
	return NULL;
}

IDisplayWindow* MainWindow::CreateQueryWindow(const String& sUser)
{
	_TRACE("MainWindow(0x%08X)::CreateQueryWindow(\"%s\")", this, sUser.Str());

	QueryWindow* pWindow = new QueryWindow;
	pWindow->SetMainWindow(this);
	pWindow->SetUser(sUser);

	pWindow->Create();

	return pWindow;
}

IDisplayWindow* MainWindow::GetDisplayWindow(const String& sKey)
{
	for(UINT i = 0; i < m_vecChildren.Size(); ++i)
	{
		ITabWindow* pWindow = m_vecChildren[i];
		_ASSERTE(pWindow != NULL);

		if(pWindow->GetTabType() == ITabWindow::TABWINDOWTYPE_CHANNEL || pWindow->GetTabType() == ITabWindow::TABWINDOWTYPE_QUERY)
		{
			DisplayWindow* pDisplay = static_cast<DisplayWindow*>(pWindow);
			_ASSERTE(pDisplay);
			if(sKey.Compare(pDisplay->GetKey(), false))
			{
				return pDisplay;
			}
		}

	}
	return NULL;
}

void MainWindow::SetDefaultWindow(IDisplayWindow* pWindow)
{
	_TRACE("MainWindow(0x%08X)::SetDefaultWindow(0x%08X)", this, pWindow);
	m_pDefaultWindow = pWindow;
}

void MainWindow::DispatchEvent(const NetworkEvent& networkEvent)
{
	//_TRACE("MainWindow(0x%08X)::DispatchEvent(\"%s\")", this, networkEvent.GetEvent().Str());

	String sKey = GetEventKey(networkEvent);
	IDisplayWindow* pWindow = GetDisplayWindow(sKey);
	if(pWindow == NULL)
	{
		pWindow = m_pDefaultWindow;
		_ASSERTE(pWindow != NULL);
	}

	if(pWindow != NULL)
	{
		pWindow->OnEvent(networkEvent);
	}
}

void MainWindow::DispatchToChannels(const NetworkEvent& event)
{
	_TRACE("MainWindow(0x%08X)::DispatchToChannels(\"%s\")", this, event.GetEvent().Str());

	for(UINT i = 0; i < m_vecChildren.Size(); ++i)
	{
		ITabWindow* pWindow = m_vecChildren[i];
		_ASSERTE(pWindow != NULL);

		if(pWindow->GetTabType() == ITabWindow::TABWINDOWTYPE_CHANNEL)
		{
			ChannelWindow* pDisplay = static_cast<ChannelWindow*>(pWindow);
			_ASSERTE(pDisplay);

			pDisplay->OnEvent(event);
		}
	}
}

void MainWindow::DispatchToChannelsWithUser(const NetworkEvent& event, const String& sUser)
{
	_TRACE("MainWindow(0x%08X)::DispatchToChannelsWithUser(\"%s\", \"%s\")", this, event.GetEvent().Str(), sUser.Str());


	for(UINT i = 0; i < m_vecChildren.Size(); ++i)
	{
		ITabWindow* pWindow = m_vecChildren[i];
		_ASSERTE(pWindow != NULL);

		if(pWindow->GetTabType() == ITabWindow::TABWINDOWTYPE_CHANNEL)
		{
			ChannelWindow* pDisplay = static_cast<ChannelWindow*>(pWindow);
			_ASSERTE(pDisplay);

			if(pDisplay->HasUser(sUser))
			{
				pDisplay->OnEvent(event);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Window Procedure
/////////////////////////////////////////////////////////////////////////////

LRESULT MainWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLEMSGRET(WM_CREATE, OnCreate);
		HANDLEMSG(WM_CLOSE, OnClose);
		HANDLEMSG(WM_DESTROY, OnDestroy);
		HANDLEMSG(WM_COMMAND, OnCommand);
		HANDLEMSG(WM_NOTIFY, OnNotify);
		HANDLEMSG(WM_SIZE, OnSize);
		HANDLEMSG(WM_SETTINGCHANGE, OnSettingChange);
		HANDLEMSG(WM_ACTIVATE, OnActivate);
		HANDLEMSG(WM_INITMENUPOPUP, OnInitMenuPopup);
		HANDLEMSG(WM_HELP, OnHelp);
		HANDLEMSG(WM_TIMER, OnTimer);
		default:
			return DefWindowProc(m_hwnd, msg, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//	Message Handling
/////////////////////////////////////////////////////////////////////////////

LRESULT MainWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
	// Create menu bar
	SHMENUBARINFO mbi = {sizeof(SHMENUBARINFO)};

	mbi.hwndParent = m_hwnd;
	mbi.hInstRes   = GetModuleHandle(NULL);
	mbi.nToolBarId = IDR_MENUBAR;
	mbi.nBmpId     = IDB_TOOLBAR;
	mbi.cBmpImages = 1;

	BOOL bCreateMenuOk = SHCreateMenuBar(&mbi);
	_ASSERTE(bCreateMenuOk != FALSE);

	m_hMenuBar = mbi.hwndMB;
	_ASSERTE(m_hMenuBar != NULL);
	SendMessage(m_hMenuBar, TB_SETSTATE, ID_SAY, MAKELONG(TBSTATE_ENABLED, 0));

	HRESULT hr = m_InputBar.Create(m_hMenuBar, m_hwnd, ID_SAY);
	_ASSERTE(SUCCEEDED(hr));

	hr = m_TabStrip.Create(m_hwnd, _T("i r teh strippar!"), WS_CHILD | WS_VISIBLE, 
		IDC_TABSTRIP, 0, 0, 240, 100);
	_ASSERTE(SUCCEEDED(hr));

	m_TabStrip.SetFont(g_Options.GetControlFont(), false);

	m_hServerMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_STATUSMENU));
	_ASSERTE(m_hServerMenu != NULL);

	SetTimer(m_hwnd, PI_TIMERID_IDLEPING, PI_PING_TIMERFREQ, NULL);

	return 0;
}

void MainWindow::OnClose(WPARAM wParam, LPARAM lParam)
{
	m_pIdent->Stop();

	if(m_pTransport->IsOpen())
	{
		m_pTransport->Close();
	}

	DestroyWindow(m_hwnd);
}

void MainWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	KillTimer(m_hwnd, PI_TIMERID_IDLEPING);

	DestroyMenu(m_hServerMenu);
	m_hServerMenu = NULL;

	DestroyWindow(m_hMenuBar);
	m_hMenuBar = NULL;

	PostQuitMessage(0);

	Window::OnDestroy(wParam, lParam);
}

void MainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
		case ID_IRC_EXIT:
			PostMessage(m_hwnd, WM_CLOSE, 0, 0);
		break;
		case ID_IRC_OPTIONS:
		{
			Options tmpOptions = g_Options;

			OptionDlg dlg(&tmpOptions);

			// Make sure taskbar is visible during dialog
			SHFullScreen(m_hwnd, SHFS_SHOWTASKBAR);

			int ret = dlg.DoModal(m_hwnd);
			if(ret == IDOK)
			{
				g_Options = tmpOptions;
				HRESULT hr = g_Options.Save(POCKETIRC_REG_HKEY_ROOT, POCKETIRC_REG_KEY_NAME);
				_ASSERTE(SUCCEEDED(hr));

				if(FAILED(hr))
				{
					MessageBoxFmt(m_hwnd, _T("There was an error saving settings\r\nError: 0x%08X"), APP_NAME, 
						MB_OK | MB_ICONEXCLAMATION, hr);
				}

				if(m_pTransport->IsOpen() && g_Options.GetIdentEnable() && !m_pIdent->IsRunning())
				{
					m_pIdent->Start(g_Options.GetIdentPort(), g_Options.GetIdentUser());
				}
				else if(!g_Options.GetIdentEnable() && m_pIdent->IsRunning())
				{
					m_pIdent->Stop();
				}

				Options::UpdateResourceCache();
			}
			OptionsChanged();
		}
		break;
		case ID_IRC_CONNECT:
			if(m_pTransport->IsOpen())
			{
				Disconnect();
			}
			else
			{
				Connect();
			}
		break;
		case ID_IRC_VIEW_FULLSCREEN:
			g_Options.SetFullScreen(!g_Options.GetFullScreen());
			OptionsChanged();
		break;
		case ID_IRC_VIEW_HIDE:
		{
			SetWindowPos(m_hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		}
		break;
		case ID_IRC_VIEW_DCCLIST:
			m_pDCCHandler->ShowDCCListWindow(!m_pDCCHandler->IsDCCListWindowVisible());
		break;
		case ID_IRC_HELP:
			OnHelp(0, 0);
		break;
		case ID_IRC_CHANNELS:
		{

			Options tmpOptions = g_Options;
			ChannelsDlg dlg(m_pSession, &tmpOptions);

			SHFullScreen(m_hwnd, SHFS_SHOWTASKBAR);

			int ret = dlg.DoModal(m_hwnd);
			if(ret == IDOK)
			{
				g_Options = tmpOptions;
				HRESULT hr = g_Options.Save(POCKETIRC_REG_HKEY_ROOT, POCKETIRC_REG_KEY_NAME);
				_ASSERTE(SUCCEEDED(hr));

				if(FAILED(hr))
				{
					MessageBoxFmt(m_hwnd, _T("There was an error saving settings\r\nError: 0x%08X"), APP_NAME, 
						MB_OK | MB_ICONEXCLAMATION, hr);
				}
			}
		}
		break;
		case InputBar::IDC_INPUTBOX:
			// Ignore EN_*
		break;
		case ID_SAY:
		{
			if(!ProcessSlashCommand())
			{
				UINT index = 0;
				if(m_TabStrip.GetCurSel(&index))
				{
					LPARAM lParam = 0;
					if(m_TabStrip.GetTabParam(index, &lParam))
					{
						ITabWindow* pWindow = (ITabWindow*)lParam;
						_ASSERTE(pWindow != NULL);

						pWindow->DoDefaultAction();
					}
				}
			}
		}
		break;
		default:
		{
			// HACK : Send unknown commands to status window...

			LPARAM lParam = 0;
			if(m_TabStrip.GetTabParam(0, &lParam))
			{
				ITabWindow* pWindow = (ITabWindow*)lParam;
				_ASSERTE(pWindow != NULL);

				SendMessage(pWindow->GetTabWindow(), WM_COMMAND, wParam, lParam);
			}
		}
	}
}

void MainWindow::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* pnmh = (NMHDR*)lParam;
	_TRACE("WM_NOTIFY (0x%08X)", pnmh->code);
	switch(pnmh->code)
	{
		case TBN_BEGINDRAG:
		{
			_TRACE("...TBN_BEGINDRAG");
		}
		break;
		case TBN_CUSTHELP:
			_TRACE("...TBN_CUSTHELP");
			if(m_bConnected && (m_hTabMenu != NULL))
			{
				RECT rcToolbar;
				GetWindowRect(m_hMenuBar, &rcToolbar);

				RECT rcSayButton;
				SendMessage(m_hMenuBar, TB_GETRECT, ID_SAY, (LPARAM)&rcSayButton);

				UINT index = 0;
				if(m_TabStrip.GetCurSel(&index))
				{
					LPARAM lParam = 0;
					if(m_TabStrip.GetTabParam(index, &lParam))
					{
						ITabWindow* pWindow = (ITabWindow*)lParam;
						_ASSERTE(pWindow != NULL);

						POINT pt = {rcToolbar.left + rcSayButton.right, rcToolbar.top + rcSayButton.top};
						pWindow->DoMenu(pt);
					}
				}

				SendMessage(m_hMenuBar, TB_SETSTATE, ID_SAY, MAKELONG(TBSTATE_ENABLED, 0));
			}
		break;
		case GN_CONTEXTMENU:
		{
			NMRGINFO* pnmri = (NMRGINFO*)lParam;
			_ASSERTE(pnmri != NULL);

			_TRACE("...GN_CONTEXTMENU ID(%d)", pnmri->hdr.idFrom);

			switch(pnmri->hdr.idFrom)
			{
				case IDC_TABSTRIP:
					_TRACE("...IDC_TABSTRIP");

					UINT index = 0;
					if(m_TabStrip.GetCurSel(&index))
					{
						LPARAM lParam = 0;
						if(m_TabStrip.GetTabParam(index, &lParam))
						{
							ITabWindow* pWindow = (ITabWindow*)lParam;
							_ASSERTE(pWindow != NULL);

							pWindow->DoMenu(pnmri->ptAction);
						}
					}

				break;
			}
		}	
		break;
	}
}

void MainWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	if(wParam != SIZE_MINIMIZED)
	{
		RECT rcClient;
		GetClientRect(m_hwnd, &rcClient);

		if(rcClient.right != m_rcOldClient.right || rcClient.bottom != m_rcOldClient.bottom)
		{
			int tabHeight = m_TabStrip.CalcTabHeight();

			MoveWindow(m_TabStrip.GetWindow(), 0, rcClient.bottom - tabHeight, rcClient.right, tabHeight, TRUE);

			for(UINT i = 0; i < m_vecChildren.Size(); ++i)
			{
				MoveWindow(m_vecChildren[i]->GetTabWindow(), 0, 0, rcClient.right, rcClient.bottom - tabHeight, TRUE);
			}
		}
		CopyMemory(&m_rcOldClient, &rcClient, sizeof(RECT));
	}
}

void MainWindow::OnSettingChange(WPARAM wParam, LPARAM lParam)
{
	if(wParam == SPI_SETSIPINFO)
	{
		UpdateSize();
	}
}

void MainWindow::OnActivate(WPARAM wParam, LPARAM lParam)
{
	if(LOWORD(wParam) != WA_INACTIVE)
	{
		m_InputBar.RestoreSipState();
		UpdateSize();

		if(LOWORD(wParam) == WA_ACTIVE)
		{
			SetFocus(m_InputBar.GetWindow());
		}
	}
}

void MainWindow::OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu = (HMENU)wParam;
	_ASSERTE(hMenu != NULL);

	// Connect menu item
	MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_STRING;
	mii.dwTypeData = (m_pTransport->IsOpen() ? _T("Dis&connect") : _T("&Connect"));

	// If this fails, we aren't handling the main menu
	BOOL bSetInfoOk = SetMenuItemInfo(hMenu, ID_IRC_CONNECT, FALSE, &mii);
	if(bSetInfoOk)
	{
		CheckMenuItem(hMenu, ID_IRC_VIEW_FULLSCREEN, MF_BYCOMMAND | (g_Options.GetFullScreen() ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenu, ID_IRC_VIEW_DCCLIST, MF_BYCOMMAND | (m_pDCCHandler->IsDCCListWindowVisible() ? MF_CHECKED : MF_UNCHECKED));

		BOOL bRemoveOk = RemoveMenu(hMenu, ID_IRC_CONTEXT, MF_BYCOMMAND);
		if(bRemoveOk)
		{
			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_ENABLED | MF_STRING | MF_POPUP, (UINT)GetSubMenu(m_hServerMenu, 0), _T("&Server"));
		}
		EnableMenuItem(hMenu, 0, MF_BYPOSITION | (m_pTransport->IsOpen() ? MF_ENABLED : MF_GRAYED));
	}
}

void MainWindow::OnHelp(WPARAM wParam, LPARAM lParam)
{
	CreateProcess(_T("PegHelp.exe"), _T("PocketIRC.htm#Main_Contents"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);
}

void MainWindow::OnTimer(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case PI_TIMERID_IDLEPING:
		if(m_bConnected && m_pSession->GetIdleTime() > PI_PING_TIMEOUT)
		{
			m_pSession->Ping();
		}
		break;
	}
}

void MainWindow::Connect()
{
	if(!m_bConnected)
	{
		Host* pHost = g_Options.GetHostList().GetDefault();
		if(pHost == NULL)
		{
			MessageBox(m_hwnd, _T("There are no hosts configured to connect to.\r\nAdd a host under IRC > Options."), 
				APP_NAME, MB_OK);
		}
		else
		{
			if(!g_Options.IsRegistered())
			{
				MessageBox(m_hwnd, _T("Thanks for evaluating PocketIRC.  Please take the time to register!\r\nhttp://pocketirc.com/"), 
					APP_NAME, MB_OK);
			}
			
			if(g_Options.GetIdentEnable() && !m_pIdent->IsRunning())
			{
				m_pIdent->Start(g_Options.GetIdentPort(), g_Options.GetIdentUser());
			}

			HRESULT hr = m_pTransport->Connect(pHost->GetAddress(), pHost->GetPort());
			_ASSERTE(SUCCEEDED(hr));

		}
	}
}

void MainWindow::Disconnect()
{
	if(m_bConnected)
	{
		m_pSession->Quit(g_Options.GetQuitMsg());
	}
	m_pTransport->Close();
}
