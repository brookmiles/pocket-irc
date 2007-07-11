#ifndef _SOCKETACCEPTER_H_INCLUDED_
#define _SOCKETACCEPTER_H_INCLUDED_

#include "SocketTransport.h"

class ISocketAccepter
{
public:
	virtual bool OnAccept(HRESULT hr, SocketTransport* pTransport, LPCTSTR psz) = 0;
};

class SocketAccepter : 
	public ITransportListen,
	public IAcceptNotifySink
{
public:
	SocketAccepter();
	~SocketAccepter();

	HRESULT SetAccepter(ISocketAccepter* pAccepter);
	HRESULT SetNotify(ISocketNotify* pNotify);

// IAcceptNotifySink
	bool OnAcceptNotify(SOCKET socket, LPARAM lParam);

// ISocketListen
	HRESULT Listen(USHORT uPort);
	HRESULT Accept();
	HRESULT Close();
	bool IsOpen();

private:
	static DWORD CALLBACK AcceptThreadProc(LPVOID lpv);

	HRESULT RunAcceptNotifyThread();

	ISocketAccepter* m_pAccepter;
	ISocketNotify* m_pNotify;
	SOCKET m_socket;
};


#endif//_SOCKETACCEPTER_H_INCLUDED_
