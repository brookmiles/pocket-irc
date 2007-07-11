#include "PocketIRC.h"
#include "SocketAccepter.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

SocketAccepter::SocketAccepter()
{
	_TRACE("SocketAccepter(0x%08X)::SocketAccepter()", this);

	m_pAccepter = NULL;
	m_pNotify = NULL;
	m_socket = INVALID_SOCKET;
}

SocketAccepter::~SocketAccepter()
{
	_TRACE("SocketAccepter(0x%08X)::~SocketAccepter()", this);

	if(m_pNotify)
	{
		m_pNotify->RemoveSink((LPARAM)(IRecvNotifySink*)this);
		m_pNotify->RemoveSink((LPARAM)(IAcceptNotifySink*)this);
	}

	if(IsOpen())
	{
		Close();
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Static Thread Procs
/////////////////////////////////////////////////////////////////////////////

DWORD CALLBACK SocketAccepter::AcceptThreadProc(LPVOID lpv)
{
	_TRACE("SocketAccepter()::AcceptThreadProc(0x%08X)", lpv);

	SocketAccepter* pAccepter = (SocketAccepter*)lpv;
	_ASSERTE(pAccepter != NULL);

	SOCKET socket = pAccepter->m_socket;
	ISocketNotify* pNotify = pAccepter->m_pNotify;

	bool bDone = false;
	while(!bDone)
	{
		SOCKADDR_IN sa;
		int len = sizeof(sa);

		SOCKET accepted = accept(socket, (SOCKADDR*)&sa, &len);
		if(accepted != INVALID_SOCKET)
		{
			USES_CONVERSION;
			String sAddr = A2CT(inet_ntoa(sa.sin_addr));

			bDone = pNotify->AcceptNotify(pAccepter, accepted, (LPARAM)sAddr.Str());
		}
		else
		{
			TCHAR buf[15];
			wsprintf(buf, _T("0x%08X"), WSAGetLastError());

			pNotify->AcceptNotify(pAccepter, accepted, (LPARAM)buf);
			bDone = true;
		}
	}

	_TRACE("SocketAccepter()::AcceptThreadProc(0x%08X) Done on socket(0x%08X)", lpv, socket);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////


HRESULT SocketAccepter::RunAcceptNotifyThread()
{
	_TRACE("SocketAccepter(0x%08X)::RunAcceptNotifyThread()", this);

	HRESULT hr = E_UNEXPECTED;

	_ASSERTE(m_pAccepter != NULL);

	HANDLE hThread = CreateThread(NULL, 0, &SocketAccepter::AcceptThreadProc, this, 0, NULL);
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

HRESULT SocketAccepter::SetAccepter(ISocketAccepter *pAccepter)
{
	_TRACE("SocketAccepter(0x%08X)::SetAccepter(0x%08X)", this, pAccepter);

	m_pAccepter = pAccepter;

	return S_OK;
}

HRESULT SocketAccepter::SetNotify(ISocketNotify* pNotify)
{
	_TRACE("SocketAccepter(0x%08X)::SetNotify(0x%08X)", this, pNotify);

	if(m_pNotify)
	{
		m_pNotify->RemoveSink((LPARAM)(IRecvNotifySink*)this);
		m_pNotify->RemoveSink((LPARAM)(IAcceptNotifySink*)this);
	}

	m_pNotify = pNotify;

	if(m_pNotify)
	{
		m_pNotify->AddSink((LPARAM)(IRecvNotifySink*)this);
		m_pNotify->AddSink((LPARAM)(IAcceptNotifySink*)this);
	}
	return S_OK;
}

HRESULT SocketAccepter::Listen(USHORT uPort)
{
	_TRACE("SocketAccepter(0x%08X)::Listen(%u)", this, uPort);

	HRESULT hr = E_UNEXPECTED;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(m_socket == INVALID_SOCKET)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
	}
	else
	{
		_TRACE("... Created socket(0x%08X)", this, m_socket);
		USES_CONVERSION;

		SOCKADDR_IN sa;
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = INADDR_ANY;
		sa.sin_port = htons(uPort);

		int err = bind(m_socket, (SOCKADDR*)&sa, sizeof(sa));
		if(err == SOCKET_ERROR)
		{
			hr = HRESULT_FROM_WIN32(WSAGetLastError());
			_TRACE("... bind() FAILED(0x%08X)", hr);
		}
		else
		{
			int err = listen(m_socket, 1);
			if(err == SOCKET_ERROR)
			{
				hr = HRESULT_FROM_WIN32(WSAGetLastError());
				_TRACE("... listen() FAILED(0x%08X)", hr);
			}
			else
			{
				hr = S_OK;
			}
		}

		if(FAILED(hr))
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
	}

	return hr;	
}

HRESULT SocketAccepter::Accept()
{
	_TRACE("SocketAccepter(0x%08X)::Accept()", this);
	_ASSERTE(m_socket != INVALID_SOCKET);

	return RunAcceptNotifyThread();
}

/////////////////////////////////////////////////////////////////////////////
//	IAcceptNotifySink
/////////////////////////////////////////////////////////////////////////////

bool SocketAccepter::OnAcceptNotify(SOCKET socket, LPARAM lParam)
{
	_TRACE("SocketAccepter(0x%08X)::OnAcceptNotify(0x%08X, \"%s\")", this, socket, (LPCTSTR)lParam);
	// return true to exit accept() thread

	if(!IsOpen())
		return true;

	if(socket != INVALID_SOCKET)
	{
		SocketTransport* pTransport = new SocketTransport();
		pTransport->SetNotify(m_pNotify);
		pTransport->UseSocket(socket);

		return m_pAccepter->OnAccept(S_OK, pTransport, (LPCTSTR)lParam);
	}
	else
	{
		return m_pAccepter->OnAccept(E_FAIL, NULL, (LPCTSTR)lParam);
	}
}


/////////////////////////////////////////////////////////////////////////////
//	ITransportListen Interface
/////////////////////////////////////////////////////////////////////////////
HRESULT SocketAccepter::Close()
{
	_TRACE("SocketAccepter(0x%08X)::Close()", this);

	_ASSERTE(m_socket != INVALID_SOCKET);

	closesocket(m_socket);
	m_socket = INVALID_SOCKET;

	return S_OK;
}

bool SocketAccepter::IsOpen()
{
	return (m_socket != INVALID_SOCKET);
}
