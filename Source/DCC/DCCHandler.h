#ifndef _DCCHANDLER_H_INCLUDED_
#define _DCCHANDLER_H_INCLUDED_

#include "Core\NetworkEvent.h"
#include "Core\Session.h"
#include "GUI\MainWindow.h"

#include "Network\ISocketNotify.h"

#include "DCCListWindow.h"
#include "IDCCChat.h"
#include "DCCChatWindow.h"

class Session;

class DCCHandler : 
	public IDCCHandler,
	public INetworkEventNotify
{
public:
	DCCHandler();
	~DCCHandler();

	void SetMainWindow(MainWindow* pMainWindow);
	void SetSession(Session* pSession) { m_pSession = pSession; } 
	Session* GetSession() { return m_pSession; }
	void SetSocketNotify(ISocketNotify* pNotify) { m_pSockNotify = pNotify; }

	void UpdateSession(IDCCSession* pSession);
	DCCChatWindow* CreateChatWindow(IDCCChat* pChat);

// INetworkEventNotify
	void OnEvent(const NetworkEvent &networkEvent);

// IDCCHandler
	void ShowDCCListWindow(bool bShow);
	bool IsDCCListWindowVisible() { return m_pDCCListWindow != NULL; }

	UINT GetSessionCount();
	IDCCSession* GetSession(UINT i);

	tstring GetNick() { return m_pSession->GetNick(); }

	void RemoveSession(IDCCSession* pSession);
	void AddSession(IDCCSession* pSession);

	void Chat(const tstring& sUser);
	void Send(const tstring& sUser);

private:
	DCCListWindow* GetDCCListWindow(bool bEnsureCreated = false);
	bool OnSend(const tstring& sPrefix, const tstring& sFileName, ULONG ulAddr, USHORT usPort, ULONG ulSize);
	bool OnChat(const tstring& sPrefix, const tstring& sType, ULONG ulAddr, USHORT usPort);
	bool OnResume(const tstring& sPrefix, const tstring& sFileName, USHORT usPort, ULONG ulSize);
	bool OnAccept(const tstring& sPrefix, const tstring& sFileName, USHORT usPort, ULONG ulSize);

	MainWindow* m_pMainWindow;
	Session* m_pSession;
	ISocketNotify* m_pSockNotify;

	DCCListWindow* m_pDCCListWindow;
	std::vector<IDCCSession*> m_Sessions;
};

#endif//_DCCHANDLER_H_INCLUDED_
