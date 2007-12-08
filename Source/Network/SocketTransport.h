#ifndef _SOCKETTRANSPORT_H_INCLUDED_
#define _SOCKETTRANSPORT_H_INCLUDED_

#include "ITransport.h"
#include "ISocketNotify.h"

class SocketTransport : 
	public ITransport,
	public ITransportWrite,
	public IRecvNotifySink
{
public:
	SocketTransport();
	~SocketTransport();

	static HRESULT Startup();
	static HRESULT Cleanup();

	static TCHAR* GetWSAErrorMessage(DWORD err);

	HRESULT SetReader(ITransportReader* pReader);
	HRESULT SetNotify(ISocketNotify* pNotify);

	void UseSocket(SOCKET socket);
	HRESULT Read();

// IRecvNotifySink
	bool OnRecvNotify(int iRecv, LPARAM lParam);
	bool OnConnectNotify(HRESULT hr, LPARAM lParam);

// ITransportWrite
	HRESULT Write(BYTE *pData, UINT nSize);

// ITransport
	HRESULT Connect(const tstring& sHostName, USHORT uPort);
	HRESULT Close();
	bool IsOpen();
	tstring GetLocalAddress();

private:
	static DWORD CALLBACK ThreadProc(LPVOID lpv);
	HRESULT RunRecvNotifyThread();

	static DWORD CALLBACK ConnectThreadStub(LPVOID lpv);
	DWORD ConnectThreadProc();

	HRESULT ConnectTryAddr(ULONG ulAddr, USHORT usPort);

	HRESULT AutoConnect();

	ITransportReader* m_pReader;
	ISocketNotify* m_pNotify;
	SOCKET m_socket;

	tstring m_sHostName;
	USHORT m_uPort;
};

#endif//_SOCKETTRANSPORT_H_INCLUDED_
