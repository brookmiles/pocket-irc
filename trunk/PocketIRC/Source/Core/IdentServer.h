#ifndef _IDENTSERVER_H_INCLUDED_
#define _IDENTSERVER_H_INCLUDED_

#include "Network\LineBuffer.h"

#include "Network\SocketTransport.h"
#include "Network\SocketAccepter.h"

#include "Network\ISocketNotify.h"

#include "NetworkEvent.h"
#include "IIdentServer.h"

// LineBuffer derives from and partially implements ITransportReader
class IdentServer :
	public LineBuffer,
	public IIdentServer,
	public ISocketAccepter
{
public:
	IdentServer();
	~IdentServer();

	void SetSocketNotify(ISocketNotify* pSockNotify);
	void SetEventNotify(INetworkEventNotify* pNotify);

// IIdentServer
	HRESULT Start(USHORT usPort, const String& sUserName);
	bool IsRunning();
	void Stop();

// ISocketAccepter
	bool OnAccept(HRESULT hr, SocketTransport* pTransport, LPCTSTR psz);

// ITransportReader
	void OnConnect(HRESULT hr, LPCTSTR pszError);
	void OnError(HRESULT hr);
	void OnClose();
// LineBuffer implements ITransportReader::OnRead()
// LineBuffer
	void OnLineRead(LPCTSTR pszLine);

private:
	String m_sUserName;
	USHORT m_uPort;
	SocketAccepter m_sockAccepter;
	SocketTransport* m_pTransport;
	INetworkEventNotify* m_pNotify;
};

#endif//_IDENTSERVER_H_INCLUDED_
