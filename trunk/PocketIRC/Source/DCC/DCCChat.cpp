#include "PocketIRC.h"
#include "DCCChat.h"

#include "IrcString.h"

#include "Core\NetworkEvent.h"
#include "Core\Reader.h"
#include "Config\Options.h"
#include "GUI\EventFormat.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

DCCChat::DCCChat()
{
	_TRACE("DCCChat(0x%08X)::DCCChat()", this);

	m_pDCCHandler = NULL;

	m_state = DCC_STATE_ERROR;
	m_bIncoming = false;

	m_uRemotePort = 0;
	m_uLocalPort = 0;

	m_pTransport = NULL;
	m_sockAccepter.SetAccepter(this);
	m_sockAccepter.SetNotify(ISocketNotify::Instance);

	m_pChatWindow = NULL;
}

DCCChat::~DCCChat()
{
	_TRACE("DCCChat(0x%08X)::~DCCChat()", this);

	if(m_pChatWindow)
	{
		m_pChatWindow->Close();
		m_pChatWindow = NULL;
	}
	delete m_pTransport;
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

void DCCChat::IncomingRequest(const String& sRemoteUser, ULONG ulRemoteAddress, USHORT ulRemotePort)
{
	USES_CONVERSION;

	m_state = DCC_STATE_REQUEST;
	m_bIncoming = true;

	IN_ADDR inad;
	inad.S_un.S_addr = htonl(ulRemoteAddress);

	m_sRemoteHost = A2CT(inet_ntoa(inad));
	m_sRemoteUser = sRemoteUser;
	m_ulRemoteAddress = ulRemoteAddress;
	m_uRemotePort = ulRemotePort;
}

bool DCCChat::OutgoingRequest(const String& sRemoteUser)
{
	USES_CONVERSION;

	m_bIncoming = false;
	m_sRemoteUser = sRemoteUser;

	HRESULT hr = E_UNEXPECTED;

	for(USHORT usPort = g_Options.GetDCCStartPort(); usPort <= g_Options.GetDCCEndPort(); usPort++)
	{
		hr = m_sockAccepter.Listen(usPort);
		if(SUCCEEDED(hr))
		{
			m_uLocalPort = usPort;

			hr = m_sockAccepter.Accept();
			_ASSERTE(SUCCEEDED(hr));

			if(FAILED(hr))
			{
				m_sockAccepter.Close();
			}

			break;
		}
	}

	if(SUCCEEDED(hr))
	{
		m_pChatWindow = m_pDCCHandler->CreateChatWindow(this);
		_ASSERTE(m_pChatWindow != NULL);

		NetworkEvent event(SYS_EVENT_WAITCONNECT, 1, &sRemoteUser);
		m_pChatWindow->OnEvent(event);

		m_state = DCC_STATE_WAITING;
	}
	else
	{
		_ASSERTE(SUCCEEDED(hr));
		m_state = DCC_STATE_ERROR;
	}
	return SUCCEEDED(hr);
}

/////////////////////////////////////////////////////////////////////////////
//	IDCCSession Interface
/////////////////////////////////////////////////////////////////////////////

void DCCChat::Accept()
{
	_TRACE("DCCChat(0x%08X)::Accept()", this);
	_ASSERTE(m_state == DCC_STATE_REQUEST);

	if(m_state == DCC_STATE_REQUEST)
	{
		m_pTransport = new SocketTransport();
		m_pTransport->SetReader(this);
		m_pTransport->SetNotify(ISocketNotify::Instance);

		HRESULT hr = m_pTransport->Connect(m_sRemoteHost, m_uRemotePort);
		_ASSERTE(SUCCEEDED(hr));

		if(SUCCEEDED(hr))
		{
			m_pChatWindow = m_pDCCHandler->CreateChatWindow(this);
			_ASSERTE(m_pChatWindow != NULL);

			m_state = DCC_STATE_CONNECTING;
		}
		else
		{
			m_state = DCC_STATE_ERROR;
		}
	}

	m_pDCCHandler->UpdateSession(this);
}

void DCCChat::Close()
{
	_TRACE("DCCChat(0x%08X)::Close()", this);

	if(m_pTransport && m_pTransport->IsOpen())
	{
		m_pTransport->Close();
		delete m_pTransport;
		m_pTransport = NULL;
	}

	if(m_sockAccepter.IsOpen())
	{
		m_sockAccepter.Close();
	}

	m_pDCCHandler->RemoveSession(this);
}

String DCCChat::GetDescription()
{
	String str = m_bIncoming ? _T("CHAT From ") : _T("CHAT To ");
	str += GetPrefixNick(m_sRemoteUser);

	return str;
}

String DCCChat::GetRemoteUser()
{
	return GetPrefixNick(m_sRemoteUser);
}

/////////////////////////////////////////////////////////////////////////////
// ITransportReader
/////////////////////////////////////////////////////////////////////////////
void DCCChat::OnConnect(HRESULT hr, LPCTSTR pszError)
{
	_TRACE("DCCChat(0x%08X)::OnConnect(0x%08X, \"%s\")", this, hr, pszError);

	NetworkEvent event;
	if(FAILED(hr))
	{
		m_state = DCC_STATE_ERROR;

		TCHAR buf[20] = _T("");
		wsprintf(buf, _T("0x%08X"), hr);

		event.SetEventID(SYS_EVENT_CONNECTFAILED);
		event.AddParam(buf);
	}
	else
	{
		if(hr == S_OK)
		{
			m_state = DCC_STATE_CONNECTED;

			event.SetEventID(SYS_EVENT_CONNECTED);
			event.AddParam(pszError);
		}
		else
		{
			event.SetEventID(SYS_EVENT_TRYCONNECT);
			event.AddParam(pszError);
		}
	}

	m_pChatWindow->OnEvent(event);

	m_pDCCHandler->UpdateSession(this);
}

void DCCChat::OnError(HRESULT hr)
{
	_TRACE("DCCChat(0x%08X)::OnError(0x%08X)", this, hr);

	m_state = DCC_STATE_ERROR;
	m_pDCCHandler->UpdateSession(this);
}

void DCCChat::OnClose()
{
	_TRACE("DCCChat(0x%08X)::OnClose()", this);

	NetworkEvent event(SYS_EVENT_CLOSE, 0);
	m_pChatWindow->OnEvent(event);

	m_state = DCC_STATE_CLOSED;
	m_pDCCHandler->UpdateSession(this);
}

/////////////////////////////////////////////////////////////////////////////
// LineBuffer
/////////////////////////////////////////////////////////////////////////////

void DCCChat::OnLineRead(LPCTSTR pszLine)
{
	_TRACE("DCCChat(0x%08X)::OnLineRead(\"%s\")", this, pszLine);

	String sFakeLine;
	sFakeLine.Reserve(POCKETIRC_MAX_IRC_LINE_LEN);

	sFakeLine += _T(":");
	sFakeLine += m_sRemoteUser;
	sFakeLine += _T(" PRIVMSG ");
	sFakeLine += m_pDCCHandler->GetNick();
	sFakeLine += _T(" :");
	sFakeLine += pszLine;

	NetworkEvent event;
	Reader::ParseEvent(event, sFakeLine.Str());
	event.SetIncoming(true);

	m_pChatWindow->OnEvent(event);
}

/////////////////////////////////////////////////////////////////////////////
//	ISocketAccepter Interface
/////////////////////////////////////////////////////////////////////////////

bool DCCChat::OnAccept(HRESULT hr, SocketTransport* pTransport, LPCTSTR psz)
{
	_TRACE("DCChat(0x%08X)::OnAccept(0x%08X, 0x%08X, \"%s\")", this, hr, pTransport, psz);
	// return true to stop accept thread loop

	NetworkEvent event;
	if(FAILED(hr))
	{
		m_state = DCC_STATE_ERROR;

		TCHAR buf[20] = _T("");
		wsprintf(buf, _T("0x%08X"), hr);

		event.SetEventID(SYS_EVENT_CONNECTFAILED);
		event.AddParam(buf);
	}
	else
	{
		_ASSERTE(pTransport != NULL);
		_ASSERTE(m_pTransport == NULL);

		m_state = DCC_STATE_CONNECTED;
		m_pDCCHandler->UpdateSession(this);

		m_pTransport = pTransport;

		m_pTransport->SetReader(this);
		m_pTransport->Read();


		m_state = DCC_STATE_CONNECTED;

		event.SetEventID(SYS_EVENT_CONNECTED);
		event.AddParam(psz);
	}

	m_pChatWindow->OnEvent(event);

	m_pDCCHandler->UpdateSession(this);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// IDCCChat
/////////////////////////////////////////////////////////////////////////////

void DCCChat::Say(const String& str)
{
	_TRACE("DCCChat(0x%08X)::Say(\"%s\")", this, str.Str());

	if(m_pTransport && m_pTransport->IsOpen())
	{
		USES_CONVERSION;
		const char* msg = T2CA(str.Str());
		m_pTransport->Write((BYTE*)msg, strlen(msg));
		m_pTransport->Write((BYTE*)"\r\n", 2);

		NetworkEvent event(IRC_CMD_PRIVMSG, 2, &GetPrefixNick(m_sRemoteUser), &str);
		m_pChatWindow->OnEvent(event);
	}
	else
	{
		MessageBeep(-1);
	}
}

void DCCChat::Act(const String& str)
{
	_TRACE("DCCChat(0x%08X)::Act(\"%s\")", this, str.Str());

	if(m_pTransport && m_pTransport->IsOpen())
	{
		USES_CONVERSION;
		const char* msg = T2CA(str.Str());
		m_pTransport->Write((BYTE*)"\x01" "ACTION ", 8);
		m_pTransport->Write((BYTE*)msg, strlen(msg));
		m_pTransport->Write((BYTE*)"\x01\r\n", 3);

		NetworkEvent event(IRC_CTCP_ACTION, 2, &GetPrefixNick(m_sRemoteUser), &str);
		m_pChatWindow->OnEvent(event);
	}
	else
	{
		MessageBeep(-1);
	}
}

void DCCChat::CloseChat()
{
	_TRACE("DCCChat(0x%08X)::CloseChat()", this);
	Close();
}
