#include "PocketIRC.h"
#include "SocketTransport.h"

#include <initguid.h>
#include <connmgr.h>

const int PI_NET_OPENTIMEOUT = 30000;

HANDLE g_hConnectionReq = NULL;

struct _WSA_ERROR_MSGS {
	DWORD code;
	TCHAR* msg;
} WSA_ERROR_MSGS[] = {
	{ WSAEINTR,                _T("A blocking operation was interrupted by a call to WSACancelBlockingCall.") },
	{ WSAEBADF,                _T("The file handle supplied is not valid.") },
	{ WSAEACCES,               _T("An attempt was made to access a socket in a way forbidden by its access permissions.") },
	{ WSAEFAULT,               _T("The system detected an invalid pointer address in attempting to use a pointer argument in a call.") },
	{ WSAEINVAL,               _T("An invalid argument was supplied.") },
	{ WSAEMFILE,               _T("Too many open sockets.") },
	{ WSAEWOULDBLOCK,          _T("A non-blocking socket operation could not be completed immediately.") },
	{ WSAEINPROGRESS,          _T("A blocking operation is currently executing.") },
	{ WSAEALREADY,             _T("An operation was attempted on a non-blocking socket that already had an operation in progress.") },
	{ WSAENOTSOCK,             _T("An operation was attempted on something that is not a socket.") },
	{ WSAEDESTADDRREQ,         _T("A required address was omitted from an operation on a socket.") },
	{ WSAEMSGSIZE,             _T("A message on a datagram socket was larger than the message buffer or other network limit.") },
	{ WSAEPROTOTYPE,           _T("A protocol was specified in the socket function call that does not support the semantics of the socket type requested.") },
	{ WSAENOPROTOOPT,          _T("An unknown, invalid, or unsupported option or level was specified in a getsockopt or setsockopt call.") },
	{ WSAEPROTONOSUPPORT,      _T("The requested protocol has not been configured into the system, or no implementation for it exists.") },
	{ WSAESOCKTNOSUPPORT,      _T("The support for the specified socket type does not exist in this address family.") },
	{ WSAEOPNOTSUPP,           _T("The attempted operation is not supported for the type of object referenced.") },
	{ WSAEPFNOSUPPORT,         _T("The protocol family has not been configured into the system or no implementation for it exists.") },
	{ WSAEAFNOSUPPORT,         _T("An address incompatible with the requested protocol was used.") },
	{ WSAEADDRINUSE,           _T("The socket address is already in use.") },
	{ WSAEADDRNOTAVAIL,        _T("The requested address is not valid in its context.") },
	{ WSAENETDOWN,             _T("Network is down.") },
	{ WSAENETUNREACH,          _T("Network is unreachable.") },
	{ WSAENETRESET,            _T("The connection was reset.") },
	{ WSAECONNABORTED,         _T("Established connection was aborted.") },
	{ WSAECONNRESET,           _T("Existing connection was forcibly closed by the remote host.") },
	{ WSAENOBUFS,              _T("System lacked sufficient buffer space or a queue was full.") },
	{ WSAEISCONN,              _T("A connect request was made on an already connected socket.") },
	{ WSAENOTCONN,             _T("Socket is not connected.") },
	{ WSAESHUTDOWN,            _T("Socket had already been shut down in that direction.") },
	{ WSAETOOMANYREFS,         _T("Too many references to some kernel object.") },
	{ WSAETIMEDOUT,            _T("The connection timed out.") },
	{ WSAECONNREFUSED,         _T("The connection was refused.") },
	{ WSAELOOP,                _T("Cannot translate name.") },
	{ WSAENAMETOOLONG,         _T("Name component or name was too long.") },
	{ WSAEHOSTDOWN,            _T("A socket operation failed because the destination host was down.") },
	{ WSAEHOSTUNREACH,         _T("A socket operation was attempted to an unreachable host.") },
	{ WSAENOTEMPTY,            _T("Cannot remove a directory that is not empty.") },
	{ WSAEPROCLIM,             _T("A Windows Sockets implementation may have a limit on the number of applications that may use it simultaneously.") },
	{ WSAEUSERS,               _T("Ran out of quota.") },
	{ WSAEDQUOT,               _T("Ran out of disk quota.") },
	{ WSAESTALE,               _T("File handle reference is no longer available.") },
	{ WSAEREMOTE,              _T("Item is not available locally.") },
	{ WSASYSNOTREADY,          _T("WSAStartup cannot function at this time because the underlying system it uses to provide network services is currently unavailable.") },
	{ WSAVERNOTSUPPORTED,      _T("The Windows Sockets version requested is not supported.") },
	{ WSANOTINITIALISED,       _T("Either the application has not called WSAStartup, or WSAStartup failed.") },
	{ WSAEDISCON,              _T("The remote party has initiated a graceful shutdown sequence.") },
	{ WSAENOMORE,              _T("No more results can be returned by WSALookupServiceNext.") },
	{ WSAECANCELLED,           _T("A call to WSALookupServiceEnd was made while a call was still processing. The call has been canceled.") },
	{ WSAEINVALIDPROCTABLE,    _T("The procedure call table is invalid.") },
	{ WSAEINVALIDPROVIDER,     _T("The requested service provider is invalid.") },
	{ WSAEPROVIDERFAILEDINIT,  _T("The requested service provider could not be loaded or initialized.") },
	{ WSASYSCALLFAILURE,       _T("A system call that should never fail has failed.") },
	{ WSASERVICE_NOT_FOUND,    _T("The service cannot be found in the specified name space.") },
	{ WSATYPE_NOT_FOUND,       _T("The specified class was not found.") },
	{ WSA_E_NO_MORE,           _T("No more results can be returned by WSALookupServiceNext.") },
	{ WSA_E_CANCELLED,         _T("A call to WSALookupServiceEnd was made while a call was still processing. The call has been canceled.") },
	{ WSAEREFUSED,             _T("A database query failed because it was actively refused.") },
	{ WSAHOST_NOT_FOUND,       _T("No such host is known.") },
	{ WSATRY_AGAIN,            _T("The local server did not receive a response from an authoritative server, this is usually temporary.") },
	{ WSANO_RECOVERY,          _T("A non-recoverable error occurred during a database lookup.") },
	{ WSANO_DATA,              _T("The requested name is valid, but no data of the requested type was found.") },
};

/////////////////////////////////////////////////////////////////////////////
//	Static Initialisation
/////////////////////////////////////////////////////////////////////////////

HRESULT SocketTransport::Startup()
{
	_TRACE("SocketTransport::Startup()");
	HRESULT hr = E_UNEXPECTED;

	WSADATA wsad;
	int err = WSAStartup(MAKEWORD(1, 1), &wsad);
	if(err != 0)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
	}
	else
	{
		hr = S_OK;
	}
	return hr;
}

HRESULT SocketTransport::Cleanup()
{
	_TRACE("SocketTransport::Cleanup()");

	HRESULT hr = E_UNEXPECTED;

	int err = WSACleanup();
	if(err != 0)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
	}
	else
	{
		hr = S_OK;
	}
	return hr;
}

TCHAR* SocketTransport::GetWSAErrorMessage(DWORD err)
{
	for(int i = 0; i < sizeof(WSA_ERROR_MSGS)/sizeof(WSA_ERROR_MSGS[0]); ++i)
	{
		if(err == WSA_ERROR_MSGS[i].code)
		{
			return WSA_ERROR_MSGS[i].msg;
		}
	}
	return _T("Unknown Error.");
}

HRESULT SocketTransport::AutoConnect()
{
	_TRACE("SocketTransport::AutoConnect()");

	HRESULT hr = E_UNEXPECTED;

	if (g_hConnectionReq)
	{
		ConnMgrReleaseConnection(g_hConnectionReq, TRUE);
		g_hConnectionReq = NULL;
	}

	HANDLE hConnMgr = ConnMgrApiReadyEvent();
	DWORD dwWait = WaitForSingleObject(hConnMgr, PI_NET_OPENTIMEOUT);
	CloseHandle(hConnMgr);

	if (WAIT_TIMEOUT == dwWait)
	{
		hr = E_FAIL;
	}
	else
	{
		DWORD dwStatus = 0;
		CONNMGR_CONNECTIONINFO ci = {sizeof(CONNMGR_CONNECTIONINFO)};
		ci.dwParams         = CONNMGR_PARAM_GUIDDESTNET;          // GUID for network is valid
		ci.dwPriority       = CONNMGR_PRIORITY_USERINTERACTIVE;   // Priority: User is waiting
		ci.bExclusive       = FALSE;                              // We wish to share this connection with other apps.
		ci.guidDestNet      = IID_DestNetInternet;                // GUID returned by mapping URL to a connection

		//m_pNotify->ConnectNotify(this, S_FALSE, (LPARAM)_T("ConnMgrEstablishConnectionSync()"));
		_TRACE("...ConnMgrEstablishConnectionSync()");
		hr = ConnMgrEstablishConnectionSync(&ci, &g_hConnectionReq, PI_NET_OPENTIMEOUT, &dwStatus);

		TCHAR* msg = 0;
		TCHAR buf[200];

		switch (dwStatus)
		{
		case CONNMGR_STATUS_UNKNOWN:
			msg = _T("Connection status is UNKNOWN.");
			break;
		case CONNMGR_STATUS_CONNECTED:
			msg = _T("The connection is up.");
			break;
		case CONNMGR_STATUS_DISCONNECTED:
			msg = _T("The connection has been disconnected.");
			break;
		case CONNMGR_STATUS_WAITINGFORPATH:
			msg = _T("A path to the destination exists, but is not presently available.");
			break;
		case CONNMGR_STATUS_WAITINGFORRESOURCE:
			msg = _T("Another client is using resources that this connection requires.");
			break;
		case CONNMGR_STATUS_WAITINGFORPHONE:
			msg = _T("A voice call is in progress and using resources that this connection requires.");
			break;
		case CONNMGR_STATUS_WAITINGFORNETWORK:
			msg = _T("The device is waiting for a task with a higher priority to connect.");
			break;
		case CONNMGR_STATUS_NOPATHTODESTINATION:
			msg = _T("No path to the destination could be found.");
			break;
		case CONNMGR_STATUS_CONNECTIONFAILED:
			msg = _T("The connection failed.");
			break;
		case CONNMGR_STATUS_CONNECTIONCANCELED:
			msg = _T("User aborted.");
			break;
		case CONNMGR_STATUS_WAITINGCONNECTION:
			msg = _T("The device is attempting to connect.");
			break;
		case CONNMGR_STATUS_WAITINGCONNECTIONABORT:
			msg = _T("The device is aborting the connection attempt.");
			break;
		case CONNMGR_STATUS_WAITINGDISCONNECTION:
			msg = _T("The connection is being brought down.");
			break;
		default:
			{
				wsprintf(buf, _T("Unknown status (0x%08X)"), dwStatus);
				msg = buf;
			}
		}

		m_pNotify->ConnectNotify(this, S_FALSE, (LPARAM)msg);
	}

	return hr;
}

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

SocketTransport::SocketTransport()
{
	_TRACE("SocketTransport(0x%08X)::SocketTransport()", this);

	m_pReader = NULL;
	m_pNotify = NULL;
	m_socket = INVALID_SOCKET;

	m_uPort = 0;
}


SocketTransport::~SocketTransport()
{
	_TRACE("SocketTransport(0x%08X)::~SocketTransport()", this);

	if(m_pNotify)
	{
		m_pNotify->RemoveSink((LPARAM)(IRecvNotifySink*)this);
	}

	if(IsOpen())
	{
		Close();
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Static Thread Procs
/////////////////////////////////////////////////////////////////////////////

DWORD CALLBACK SocketTransport::ThreadProc(LPVOID lpv)
{
	_TRACE("SocketTransport()::ThreadProc(0x%08X)", lpv);

	SocketTransport* pTransport = (SocketTransport*)lpv;
	_ASSERTE(pTransport != NULL);

	SOCKET socket = pTransport->m_socket;
	ISocketNotify* pNotify = pTransport->m_pNotify;

	BYTE buf[POCKETIRC_MAX_IRC_LINE_LEN];
	bool bDone = false;
	while(!bDone)
	{
		int iRecv = recv(socket, (char*)buf, sizeof(buf), 0);
		if(iRecv > 0)
		{
			bDone = pNotify->RecvNotify(pTransport, iRecv, (LPARAM)buf);
		}
		else
		{
			pNotify->RecvNotify(pTransport, iRecv, (LPARAM)WSAGetLastError());
			bDone = true;
		}
	}

	_TRACE("SocketTransport()::ThreadProc(0x%08X) Done. Closing socket(0x%08X)", lpv, socket);
	closesocket(socket);

	return 0;
}

DWORD CALLBACK SocketTransport::ConnectThreadStub(LPVOID lpv)
{
	_TRACE("SocketTransport()::ConnectThreadStub(0x%08X)", lpv);

	SocketTransport* pTransport = (SocketTransport*)lpv;
	_ASSERTE(pTransport != NULL);

	return pTransport->ConnectThreadProc();
}

DWORD SocketTransport::ConnectThreadProc()
{
	_TRACE("SocketTransport(0x%08X)::ConnectThreadProc()", this);

	HRESULT hr = E_UNEXPECTED;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(m_socket == INVALID_SOCKET)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
	}
	else
	{
		_TRACE("SocketTransport(0x%08X)::Connect() Created socket(0x%08X)", this, m_socket);
		USES_CONVERSION;

		m_pNotify->ConnectNotify(this, S_FALSE, (LPARAM)m_sHostName.Str());

		hr = AutoConnect();
		_TRACE("...AutoConnect() returned 0x%08X", hr);

		ULONG addr = inet_addr(T2CA(m_sHostName.Str()));
		if(addr == INADDR_NONE)
		{
			HOSTENT* pHostEnt = gethostbyname(T2CA(m_sHostName.Str()));
			if(pHostEnt != NULL)
			{
				int i = 0;
				while(pHostEnt->h_addr_list[i] != NULL && m_socket != INVALID_SOCKET)
				{
					hr = ConnectTryAddr(*(ULONG*)pHostEnt->h_addr_list[i], m_uPort);

					if(SUCCEEDED(hr))
					{
						break;
					}
					else
					{
						++i;
					}
				}
			}
			else 
			{
				hr = HRESULT_FROM_WIN32(WSAGetLastError());
			}
		}
		else
		{
			hr = ConnectTryAddr(addr, m_uPort);
		}

		if(SUCCEEDED(hr))
		{
			hr = Read();
		}

		if(FAILED(hr))
		{
			_TRACE("SocketTransport(0x%08X)::Connect() FAILED!", this);

			if(m_socket != INVALID_SOCKET)
			{
				_TRACE("... Closing socket(0x%08X)", m_socket);
				closesocket(m_socket);
				m_socket = INVALID_SOCKET;
			}
		}
	}

	m_pNotify->ConnectNotify(this, hr, (LPARAM)m_sHostName.Str());
	return hr;	

}

/////////////////////////////////////////////////////////////////////////////
//	Utility Methods
/////////////////////////////////////////////////////////////////////////////

HRESULT SocketTransport::ConnectTryAddr(ULONG ulAddr, USHORT usPort)
{
	_TRACE("SocketTransport(0x%08X)::ConnectTryAddr(0x%08X, %u)", this, ulAddr, usPort);

	HRESULT hr = E_UNEXPECTED;

	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(usPort);
	sa.sin_addr.S_un.S_addr = ulAddr;

	TCHAR szConnect[200];
	wsprintf(szConnect, _T("%d.%d.%d.%d:%d"), 
		sa.sin_addr.S_un.S_un_b.s_b1, 
		sa.sin_addr.S_un.S_un_b.s_b2, 
		sa.sin_addr.S_un.S_un_b.s_b3, 
		sa.sin_addr.S_un.S_un_b.s_b4,
		usPort);

	m_pNotify->ConnectNotify(this, S_FALSE, (LPARAM)szConnect);

	int iRet = connect(m_socket, (SOCKADDR*)&sa, sizeof(sa));
	if(iRet == SOCKET_ERROR)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
	}
	else
	{
		hr = S_OK;
	}
	return hr;
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

HRESULT SocketTransport::Connect(const String& sHostName, USHORT uPort)
{
	_TRACE("SocketTransport(0x%08X)::Connect(\"%s\", %u)", this, sHostName.Str(), uPort);

	HRESULT hr = E_UNEXPECTED;

	m_sHostName = sHostName;
	m_uPort = uPort;

	HANDLE hThread = CreateThread(NULL, 0, &SocketTransport::ConnectThreadStub, this, 0, NULL);
	_ASSERTE(hThread != NULL);
	if(hThread == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		CloseHandle(hThread);
		hr = S_OK;
	}
	return hr;
}

HRESULT SocketTransport::RunRecvNotifyThread()
{
	_TRACE("SocketTransport(0x%08X)::RunRecvNotifyThread()", this);

	HRESULT hr = E_UNEXPECTED;

	_ASSERTE(m_pReader != NULL);

	HANDLE hThread = CreateThread(NULL, 0, &SocketTransport::ThreadProc, this, 0, NULL);
	_ASSERTE(hThread != NULL);
	if(hThread == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		CloseHandle(hThread);
		hr = S_OK;
	}
	return hr;
}

HRESULT SocketTransport::SetReader(ITransportReader *pReader)
{
	_TRACE("SocketTransport(0x%08X)::SetReader(0x%08X)", this, pReader);

	m_pReader = pReader;

	return S_OK;
}

HRESULT SocketTransport::SetNotify(ISocketNotify* pNotify)
{
	_TRACE("SocketTransport(0x%08X)::SetNotify(0x%08X)", this, pNotify);

	if(m_pNotify)
	{
		m_pNotify->RemoveSink((LPARAM)(IRecvNotifySink*)this);
	}

	m_pNotify = pNotify;

	if(m_pNotify)
	{
		m_pNotify->AddSink((LPARAM)(IRecvNotifySink*)this);
	}
	return S_OK;
}

void SocketTransport::UseSocket(SOCKET socket)
{
	_ASSERTE(m_socket == INVALID_SOCKET);

	m_socket = socket;

	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 0;
	setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
}

HRESULT SocketTransport::Read()
{
	return RunRecvNotifyThread();
}

/////////////////////////////////////////////////////////////////////////////
//	ISocketNotify
/////////////////////////////////////////////////////////////////////////////

bool SocketTransport::OnRecvNotify(int iRecv, LPARAM lParam)
{
	// return true to end recv thread

	if(!IsOpen())
		return true;

	if(iRecv > 0)
	{
		BYTE* pData = (BYTE*)lParam;
		_ASSERTE(pData != NULL);

		m_pReader->OnRead(pData, iRecv);
		return false;
	}
	else if(iRecv == 0)
	{
		if(IsOpen())
		{
			Close();
		}
	}
	else
	{
		if(lParam != WSAESHUTDOWN)
		{
			m_pReader->OnError(HRESULT_FROM_WIN32(lParam));
		}

		if(IsOpen())
		{
			Close();
		}
	}

	return true;
}

bool SocketTransport::OnConnectNotify(HRESULT hr, LPARAM lParam)
{
	m_pReader->OnConnect(hr, (LPCTSTR)lParam);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
//	ITransport Interface
/////////////////////////////////////////////////////////////////////////////

HRESULT SocketTransport::Write(BYTE *pData, UINT nSize)
{
	_ASSERTE(pData != NULL);

	HRESULT hr = E_UNEXPECTED;

	if(m_pReader)
	{
		int iRet = send(m_socket, (const char*)pData, nSize, 0);
		if(iRet == SOCKET_ERROR)
		{
			hr = HRESULT_FROM_WIN32(WSAGetLastError());		
		}
		else
		{
			hr = S_OK;
		}
	}

	return hr;
}

HRESULT SocketTransport::Close()
{
	_TRACE("SocketTransport(0x%08X)::Close()", this);

	_ASSERTE(m_socket != INVALID_SOCKET);

	HRESULT hr = E_UNEXPECTED;

	int iRet = shutdown(m_socket, 2);	//SD_BOTH
	if(iRet == SOCKET_ERROR)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
	}
	else
	{
		hr = S_OK;
	}

	// Recv thread may close socket as well, but in some cases it won't
	// so just to be sure.
	closesocket(m_socket);
	m_socket = INVALID_SOCKET;

	m_pReader->OnClose();

	return hr;
}

bool SocketTransport::IsOpen()
{
	return (m_socket != INVALID_SOCKET);
}

String SocketTransport::GetLocalAddress()
{
	_TRACE("SocketTransport(0x%08X)::GetLocalAddress()", this);
	String sAddr;

	if(IsOpen())
	{
		SOCKADDR_IN sa;
		int size = sizeof(sa);

		if(getsockname(m_socket, (SOCKADDR*)&sa, &size) == 0)
		{
			sAddr.Reserve(50);
			wsprintf(sAddr.Str(), _T("%d.%d.%d.%d"), 
				sa.sin_addr.S_un.S_un_b.s_b1, 
				sa.sin_addr.S_un.S_un_b.s_b2, 
				sa.sin_addr.S_un.S_un_b.s_b3, 
				sa.sin_addr.S_un.S_un_b.s_b4);
			_TRACE("... %s", sAddr.Str());
		}
	}

	return sAddr;
}
