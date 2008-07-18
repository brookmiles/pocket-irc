#ifndef _MAINWINDOW_H_INCLUDED_
#define _MAINWINDOW_H_INCLUDED_

#include "Window.h"
#include "InputBar.h"
#include "TabStrip.h"

#include "Network\ITransport.h"
#include "Core\Session.h"
#include "Core\IIdentServer.h"

#include "DCC\IDCCHandler.h"

#include "IDisplayWindow.h"

class Session;

class ITabWindow
{
public:	
	virtual HWND GetTabWindow() = 0;
	virtual const tstring GetTabTitle() = 0;

	virtual void DoMenu(POINT pt) = 0;
	virtual void DoDefaultAction() = 0;
	virtual void OnOptionsChanged() = 0;

	enum TABWINDOWTYPE
	{
		TABWINDOWTYPE_STATUS,
		TABWINDOWTYPE_QUERY,
		TABWINDOWTYPE_CHANNEL,
		TABWINDOWTYPE_DCC,
	};

	virtual TABWINDOWTYPE GetTabType() = 0;
};

class MainWindow : 
	public Window,
	public INetworkEventNotify
{
	enum MAINWINDOW_IDC { IDC_TABSTRIP = 100, IDC_TOOLBAR };
public:
	DECL_WINDOW_CLASSNAME("PocketIRCWindow");

	static bool ActivatePrevious();

	MainWindow();
	~MainWindow();

	HRESULT Init();
	HRESULT Create();
	HRESULT UnInit();

	void SetSession(Session* pSession);
	Session* GetSession() { return m_pSession; }
	void SetTransport(ITransport* pTransport);
	void SetIdentServer(IIdentServer* pIdent);
	void SetDCCHandler(IDCCHandler* pDCCHandler);
	IDCCHandler* GetDCCHandler() { return m_pDCCHandler; }

	void UpdateSize();

// MainWindow
	void AddWindowTab(ITabWindow* pWindow);
	void RemoveWindowTab(ITabWindow* pWindow);
	void SetWindowTabTitle(ITabWindow* pWindow, const tstring& sTitle);
	void SetWindowTabHighlight(ITabWindow* pWindow, int iHighlight);
	void SetWindowTabMenu(HMENU hMenu);
	void SetCurrentTarget(const tstring& sTarget);
	tstring GetCurrentTarget() { return m_sCurrentTarget; }
	tstring GetInput();
	void ClearInput();
	void InsertInput(const tstring& str);

	HWND GetWindow() { return Window::GetWindow(); }

	void OpenQuery(const tstring& user);
	void CloseQuery(const tstring& user);

// IEventHandler
	void OnEvent(const NetworkEvent &networkEvent);

private:
	SHACTIVATEINFO m_shackInfo;

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	void OnClose(WPARAM wParam, LPARAM lParam);
	void OnDestroy(WPARAM wParam, LPARAM lParam);
	void OnCommand(WPARAM wParam, LPARAM lParam);
	void OnNotify(WPARAM wParam, LPARAM lParam);
	void OnSize(WPARAM wParam, LPARAM lParam);
	void OnSettingChange(WPARAM wParam, LPARAM lParam);
	void OnActivate(WPARAM wParam, LPARAM lParam);
	void OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
	void OnHelp(WPARAM wParam, LPARAM lParam);
	void OnTimer(WPARAM wParam, LPARAM lParam);
	void OnSwitchWindow(WPARAM wParam, LPARAM lParam);

	void OptionsChanged();
	void Connect();
	void Disconnect();

	bool ProcessSlashCommand();
	void SlashQuit(const tstring& s);
	void SlashPart(const tstring& s);
	void SlashMsg(const tstring& s);
	void SlashQuery(const tstring& s);
	void SlashClose(const tstring& s);
	void SlashSay(const tstring& s);
	void SlashMe(const tstring& s);
	void SlashKick(const tstring& s);
	void SlashInvite(const tstring& s);
	void SlashNotice(const tstring& s);
	void SlashTopic(const tstring& s);
	void SlashRaw(const tstring& s);
	void SlashExit(const tstring& s);
	void SlashAway(const tstring& s);
	void SlashPing(const tstring& s);
	void SlashCTCP(const tstring& s);
	void SlashConnect(const tstring& s);
	void SlashDisconnect(const tstring& s);
	void SlashServer(const tstring& s);

	void OnConnectStateChange(const NetworkEvent &e);
	void OnJoin(const NetworkEvent& event);
	void OnPart(const NetworkEvent& event);
	void OnKick(const NetworkEvent& event);
	void OnNick(const NetworkEvent& event);
	void OnQuit(const NetworkEvent& event);
	void OnPrivMsg(const NetworkEvent& event);
	void OnRplWelcome(const NetworkEvent& event);
	void OnRplUserHost(const NetworkEvent& event);

	IDisplayWindow* CreateQueryWindow(const tstring& sUser);
	IDisplayWindow* CreateChannelWindow(const tstring& sChannel);

	IDisplayWindow* GetDisplayWindow(const tstring& sKey);
	void SetDefaultWindow(IDisplayWindow* pWindow);
	void DispatchEvent(const NetworkEvent& event);
	void DispatchToChannels(const NetworkEvent& event);
	void DispatchToChannelsWithUser(const NetworkEvent& event, const tstring& sUser);

	HWND m_hMenuBar;
	InputBar m_InputBar;
	TabStrip m_TabStrip;
	bool m_bConnected;

	std::vector<ITabWindow*> m_vecChildren;
	IDisplayWindow* m_pDefaultWindow;


	ITransport* m_pTransport;
	Session* m_pSession;
	IIdentServer* m_pIdent;
	IDCCHandler* m_pDCCHandler;

	HMENU m_hTabMenu;
	HMENU m_hServerMenu;

	RECT m_rcOldClient;
	tstring m_sCurrentTarget;
};

#endif//_MAINWINDOW_H_INCLUDED_
