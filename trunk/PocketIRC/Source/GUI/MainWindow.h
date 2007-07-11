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
	virtual const String GetTabTitle() = 0;

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
	void SetWindowTabTitle(ITabWindow* pWindow, const String& sTitle);
	void SetWindowTabHighlight(ITabWindow* pWindow, int iHighlight);
	void SetWindowTabMenu(HMENU hMenu);
	void SetCurrentTarget(const String& sTarget);
	String GetCurrentTarget() { return m_sCurrentTarget; }
	String GetInput();
	void ClearInput();
	void InsertInput(const String& str);

	HWND GetWindow() { return Window::GetWindow(); }

	void OpenQuery(const String& user);
	void CloseQuery(const String& user);

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

	void OptionsChanged();
	void Connect();
	void Disconnect();

	bool ProcessSlashCommand();
	void SlashQuit(const String& s);
	void SlashPart(const String& s);
	void SlashMsg(const String& s);
	void SlashQuery(const String& s);
	void SlashClose(const String& s);
	void SlashSay(const String& s);
	void SlashMe(const String& s);
	void SlashKick(const String& s);
	void SlashInvite(const String& s);
	void SlashNotice(const String& s);
	void SlashTopic(const String& s);
	void SlashRaw(const String& s);
	void SlashExit(const String& s);
	void SlashAway(const String& s);
	void SlashPing(const String& s);
	void SlashCTCP(const String& s);
	void SlashConnect(const String& s);
	void SlashDisconnect(const String& s);
	void SlashServer(const String& s);

	void OnConnectStateChange(const NetworkEvent &e);
	void OnJoin(const NetworkEvent& event);
	void OnPart(const NetworkEvent& event);
	void OnKick(const NetworkEvent& event);
	void OnNick(const NetworkEvent& event);
	void OnQuit(const NetworkEvent& event);
	void OnPrivMsg(const NetworkEvent& event);
	void OnRplWelcome(const NetworkEvent& event);
	void OnRplUserHost(const NetworkEvent& event);

	IDisplayWindow* CreateQueryWindow(const String& sUser);
	IDisplayWindow* CreateChannelWindow(const String& sChannel);

	IDisplayWindow* GetDisplayWindow(const String& sKey);
	void SetDefaultWindow(IDisplayWindow* pWindow);
	void DispatchEvent(const NetworkEvent& event);
	void DispatchToChannels(const NetworkEvent& event);
	void DispatchToChannelsWithUser(const NetworkEvent& event, const String& sUser);

	HWND m_hMenuBar;
	InputBar m_InputBar;
	TabStrip m_TabStrip;
	bool m_bConnected;

	Vector<ITabWindow*> m_vecChildren;
	IDisplayWindow* m_pDefaultWindow;


	ITransport* m_pTransport;
	Session* m_pSession;
	IIdentServer* m_pIdent;
	IDCCHandler* m_pDCCHandler;

	HMENU m_hTabMenu;
	HMENU m_hServerMenu;

	RECT m_rcOldClient;
	String m_sCurrentTarget;
};

#endif//_MAINWINDOW_H_INCLUDED_
