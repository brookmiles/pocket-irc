#ifndef _DCCCHAT_H_INCLUDED_
#define _DCCCHAT_H_INCLUDED_

#include "IDCCSession.h"

#include "Network\SocketTransport.h"
#include "Network\SocketAccepter.h"
#include "Network\LineBuffer.h"

#include "GUI\IDisplayWindow.h"

#include "DCCHandler.h"

#include "IDCCChat.h"

class DCCChat : 
	public IDCCSession, 
	public IDCCChat, 
	public LineBuffer, 
	public ISocketAccepter
{
public:
	DCCChat();
	~DCCChat();

	void SetDCCHandler(DCCHandler* pDCCHandler) { m_pDCCHandler = pDCCHandler; }

	void IncomingRequest(const tstring& sRemoteUser, ULONG ulRemoteAddress, USHORT ulRemotePort);
	bool OutgoingRequest(const tstring& sRemoteUser);

// IDCCChat
	void Say(const tstring& str);
	void Act(const tstring& str);
	void CloseChat();

// IDCCSession
	DCC_TYPE GetType() { return DCC_CHAT; }
	DCC_STATE GetState() { return m_state; }
	bool IsIncoming() { return m_bIncoming; }

	void Accept();
	void Close();

	tstring GetRemoteUser();
	tstring GetRemoteHost() { return m_sRemoteHost; }
	USHORT GetRemotePort() { return m_uRemotePort; }
	USHORT GetLocalPort() { return m_uLocalPort; }

	tstring GetDescription();

// ITransportReader
	void OnConnect(HRESULT hr, LPCTSTR pszError);
	void OnError(HRESULT hr);
	void OnClose();
// LineBuffer implements ITransportReader::OnRead()
// LineBuffer
	void OnLineRead(LPCTSTR pszLine);

// ISocketAccepter
	bool OnAccept(HRESULT hr, SocketTransport* pTransport, LPCTSTR psz);

private:
	DCCHandler* m_pDCCHandler;

	DCC_STATE m_state;
	bool m_bIncoming;

	tstring m_sRemoteUser;
	tstring m_sRemoteHost;
	ULONG m_ulRemoteAddress;
	USHORT m_uRemotePort;
	USHORT m_uLocalPort;

	SocketAccepter m_sockAccepter;
	SocketTransport* m_pTransport;
	DCCChatWindow* m_pChatWindow;
};

#endif//_DCCCHAT_H_INCLUDED_
