#include "PocketIRC.h"
#include "IdentServer.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

IdentServer::IdentServer()
{
	_TRACE("IdentServer(0x%08X)::IdentServer()", this);

	m_uPort = 0;
	m_pNotify = NULL;
	m_pTransport = NULL;
	m_sockAccepter.SetAccepter(this);
}


IdentServer::~IdentServer()
{
	_TRACE("IdentServer(0x%08X)::~IdentServer()", this);

	Stop();
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

void IdentServer::SetEventNotify(INetworkEventNotify* pNotify)
{
	_TRACE("IdentServer(0x%08X)::SetEventNotify(0x%08X)", this, pNotify);
	m_pNotify = pNotify;
}

void IdentServer::SetSocketNotify(ISocketNotify* pNotify)
{
	_TRACE("IdentServer(0x%08X)::SetSocketNotify(0x%08X)", this, pNotify);
	m_sockAccepter.SetNotify(pNotify);
}

HRESULT IdentServer::Start(USHORT usPort, const tstring& sUserName)
{
	_TRACE("IdentServer(0x%08X)::Start(%u, \"%s\")", this, usPort, sUserName.c_str());

	HRESULT hr = E_UNEXPECTED;

	m_sUserName = sUserName;
	m_uPort = usPort;

	hr = m_sockAccepter.Listen(usPort);
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr))
	{
		hr = m_sockAccepter.Accept();
		_ASSERTE(SUCCEEDED(hr));
	}

	return hr;
}

bool IdentServer::IsRunning()
{
	return m_sockAccepter.IsOpen();
}

void IdentServer::Stop()
{
	_TRACE("IdentServer(0x%08X)::Stop()", this);

	m_uPort = 0;
	m_sUserName.clear();

	if(m_pTransport)
	{
		if(m_pTransport->IsOpen())
		{
			m_pTransport->Close();
		}
		delete m_pTransport;
		m_pTransport = NULL;
	}

	if(m_sockAccepter.IsOpen())
	{
		m_sockAccepter.Close();
	}
}

/////////////////////////////////////////////////////////////////////////////
//	ISocketAccepter Interface
/////////////////////////////////////////////////////////////////////////////

bool IdentServer::OnAccept(HRESULT hr, SocketTransport* pTransport, LPCTSTR psz)
{
	_TRACE("IdentServer(0x%08X)::OnAccept(0x%08X, 0x%08X, \"%s\")", this, hr, pTransport, psz);
	// return true to stop accept thread loop

	if(SUCCEEDED(hr))
	{
		_ASSERTE(pTransport != NULL);
		_ASSERTE(m_pTransport == NULL);

		if(m_pNotify)
		{
			NetworkEvent event;

			event.SetEventID(SYS_EVENT_IDENT);
			event.AddParam(psz);

			m_pNotify->OnEvent(event);
		}

		m_pTransport = pTransport;
		
		m_pTransport->SetReader(this);
		m_pTransport->Read();
	}
	else
	{
		if(m_pNotify)
		{
			NetworkEvent event;

			event.SetEventID(SYS_EVENT_ERROR);
			event.AddParam(_T("IDENT accept() returned:"));
			event.AddParam(psz);

			m_pNotify->OnEvent(event);
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//	ITransportReader Interface
/////////////////////////////////////////////////////////////////////////////

void IdentServer::OnConnect(HRESULT hr, LPCTSTR pszError)
{
	_TRACE("IdentServer(0x%08X)::OnConnect(0x%08X, \"%s\")", this, hr, pszError);

	// This shouldn't ever get called.
	_ASSERTE(FALSE);
}

void IdentServer::OnError(HRESULT hr)
{
	_TRACE("IdentServer(0x%08X)::OnError(0x%08X)", this, hr);
}

void IdentServer::OnClose()
{
	_TRACE("IdentServer(0x%08X)::OnClose()", this);

	delete m_pTransport;
	m_pTransport = NULL;

	if(m_uPort)
	{
		m_sockAccepter.Accept();
	}
}

void IdentServer::OnLineRead(LPCTSTR pszLine)
{
	_TRACE("IdentServer(0x%08X)::OnLineRead(\"%s\")", this, pszLine);
	_ASSERTE(m_pTransport != NULL);

	TCHAR buf[100];
	_sntprintf(buf, sizeof(buf)/sizeof(TCHAR), _T("%s : USERID : UNKNOWN : %s\r\n"), pszLine, m_sUserName.c_str());
	buf[sizeof(buf)/sizeof(TCHAR) - 1] = '\0';

	USES_CONVERSION;
	m_pTransport->Write((BYTE*)W2CA(buf), _tcslen(buf));
	m_pTransport->Close();
}
