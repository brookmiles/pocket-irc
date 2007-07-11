#include "PocketIRC.h"
#include "DCCHandler.h"

#include "IrcString.h"
#include "Config\Options.h"
#include "DCCChat.h"
#include "DCCSend.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

DCCHandler::DCCHandler()
{
	m_pDCCListWindow = NULL;
	m_pSession = NULL;
	m_pSockNotify = NULL;
}

DCCHandler::~DCCHandler()
{

}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

void DCCHandler::SetMainWindow(MainWindow* pMainWindow)
{
	m_pMainWindow = pMainWindow;
}

void DCCHandler::UpdateSession(IDCCSession* pSession)
{
	if(m_pDCCListWindow)
	{
		m_pDCCListWindow->UpdateSession(pSession);
	}
}

DCCChatWindow* DCCHandler::CreateChatWindow(IDCCChat* pChat)
{
	DCCChatWindow* pWindow = new DCCChatWindow;

	pWindow->SetDCCHandler(this);
	pWindow->SetChat(pChat);

	pWindow->SetTitle(pChat->GetRemoteUser());
	pWindow->SetMainWindow(m_pMainWindow);
	pWindow->Create();

	return pWindow;
}

/////////////////////////////////////////////////////////////////////////////
//	IEventHandler Interface
/////////////////////////////////////////////////////////////////////////////

void DCCHandler::OnEvent(const NetworkEvent &event)
{
	if(event.GetEventID() == IRC_CTCP_DCC)
	{
		_TRACE("DCCHandler(0x%08X)::OnEvent()", this);

		const String& sParams = event.GetParam(1);
		String sCmd = sParams.GetWord(0);

		if(sCmd.Compare(_T("SEND"), false))
		{
			if(!g_Options.GetIgnoreSend())
			{
				String sFileName = sParams.GetWord(1);
				ULONG ulAddr = _tcstoul(sParams.GetWord(2).Str(), NULL, 10);
				USHORT usPort = (USHORT)_tcstoul(sParams.GetWord(3).Str(), NULL, 10);
				ULONG ulSize = _tcstoul(sParams.GetWord(4).Str(), NULL, 10);

				TCHAR* pszFile = _tcsrchr(sFileName.Str(), '/');
				if(pszFile == NULL)
					 pszFile = _tcsrchr(sFileName.Str(), '\\');
				if(pszFile != NULL && pszFile[0] != NULL)
				{
					UINT nFileName = pszFile - sFileName.Str() + 1;
					sFileName = sFileName.SubStr(nFileName + 1);
				}

				_ASSERTE(usPort != 0);
				_ASSERTE(ulAddr != 0 && ulAddr != -1);

				if(ulAddr == 0 || usPort == 0)
				{
					_TRACE("...INVALID ADDR OR PORT (%s):(%s)", sParams.GetWord(3).Str(), sParams.GetWord(4).Str());
				}
				else if(sFileName.Size() == 0)
				{
					_TRACE("...INVALID FILENAME");
				}
				else
				{
					OnSend(event.GetPrefix(), sFileName, ulAddr, usPort, ulSize);
				}
			}
		}
		else if(sCmd.Compare(_T("CHAT"), false))
		{
			if(!g_Options.GetIgnoreChat())
			{
				String sType = sParams.GetWord(1);
				ULONG ulAddr = _tcstoul(sParams.GetWord(2).Str(), NULL, 10);
				USHORT usPort = (USHORT)_tcstoul(sParams.GetWord(3).Str(), NULL, 10);

				_ASSERTE(usPort != 0);
				_ASSERTE(ulAddr != 0 && ulAddr != -1);

				if(ulAddr == 0 || usPort == 0)
				{
					_TRACE("...INVALID ADDR OR PORT (%s):(%s)", sParams.GetWord(3).Str(), sParams.GetWord(4).Str());
				}
				else if(!sType.Compare(_T("CHAT"), false))
				{
					_TRACE("...INVALID CHAT TYPE (%s)", sType.Str());
				}
				else
				{
					OnChat(event.GetPrefix(), sType, ulAddr, usPort);
				}
			}
		}
		else if(sCmd.Compare(_T("RESUME"), false) || sCmd.Compare(_T("ACCEPT"), false))
		{
			if(m_Sessions.Size())
			{
				String sFileName = sParams.GetWord(1);
				USHORT usPort = (USHORT)_tcstoul(sParams.GetWord(2).Str(), NULL, 10);
				ULONG ulSize = _tcstoul(sParams.GetWord(3).Str(), NULL, 10);

				TCHAR* pszFile = _tcsrchr(sFileName.Str(), '/');
				if(pszFile == NULL)
					 pszFile = _tcsrchr(sFileName.Str(), '\\');
				if(pszFile != NULL && pszFile[0] != NULL)
				{
					UINT nFileName = pszFile - sFileName.Str() + 1;
					sFileName = sFileName.SubStr(nFileName + 1);
				}

				_ASSERTE(usPort != 0);

				if(usPort == 0)
				{
					_TRACE("...INVALID PORT (%s)", sParams.GetWord(3).Str());
				}
				else if(sFileName.Size() == 0)
				{
					_TRACE("...INVALID FILENAME");
				}
				else
				{
					if(sCmd.Compare(_T("RESUME"), false))
					{
						OnResume(event.GetPrefix(), sFileName, usPort, ulSize);
					}
					else
					{
						OnAccept(event.GetPrefix(), sFileName, usPort, ulSize);
					}
				}
			}
		}
		else
		{
			_TRACE("...UNKNOWN COMMAND (%s)", sCmd.Str());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Command Handlers
/////////////////////////////////////////////////////////////////////////////

bool DCCHandler::OnSend(const String& sPrefix, const String& sFileName, ULONG ulAddr, USHORT usPort, ULONG ulSize)
{
	_TRACE("DCCHandler(0x%08X)::OnSend(\"%s\", \r\n\t\"%s\", 0x%08X, %u, %u)", this, sPrefix.Str(), sFileName.Str(), ulAddr, usPort, ulSize);

	DCCListWindow* pDCCWnd = GetDCCListWindow(true);
	_ASSERTE(pDCCWnd != NULL);

	DCCSend *pSend = new DCCSend();
	pSend->SetDCCHandler(this);
	pSend->IncomingRequest(sPrefix, ulAddr, usPort, sFileName, ulSize);

	m_Sessions.Append(pSend);

	pDCCWnd->AddSession(pSend);

	return true;
}

bool DCCHandler::OnChat(const String& sPrefix, const String& sType, ULONG ulAddr, USHORT usPort)
{
	_TRACE("DCCHandler(0x%08X)::OnChat(\"%s\", \r\n\t\"%s\", 0x%08X, %u)", this, sPrefix.Str(), sType.Str(), ulAddr, usPort);

	DCCListWindow* pDCCWnd = GetDCCListWindow(true);
	_ASSERTE(pDCCWnd != NULL);

	DCCChat *pChat = new DCCChat();
	pChat->SetDCCHandler(this);
	pChat->IncomingRequest(sPrefix, ulAddr, usPort);

	m_Sessions.Append(pChat);

	pDCCWnd->AddSession(pChat);

	return true;
}

bool DCCHandler::OnResume(const String& sPrefix, const String& sFileName, USHORT usPort, ULONG ulSize)
{
	for(UINT i = 0; i < m_Sessions.Size(); ++i)
	{
		IDCCSession *pSession = m_Sessions[i];
		_ASSERTE(pSession != NULL);

		String sNick = GetPrefixNick(sPrefix);
		if((pSession->GetType() == DCC_SEND) && !pSession->IsIncoming() && (pSession->GetState() == DCC_STATE_WAITING) && 
			(pSession->GetLocalPort() == usPort) && (pSession->GetRemoteUser() == sNick))
		{
			DCCSend* pDCCSend = ((DCCSend*)pSession);
			if(pDCCSend->Resume(ulSize))
			{
				TCHAR buf[POCKETIRC_MAX_IRC_LINE_LEN];
				_sntprintf(buf, sizeof(buf), _T("ACCEPT %s %u %u"), sFileName.Str(), usPort, ulSize);
				buf[sizeof(buf) - 1] = 0;

				m_pSession->CTCP(sNick, _T("DCC"), buf);
			}
		}
	}
	return true;
}

bool DCCHandler::OnAccept(const String& sPrefix, const String& sFileName, USHORT usPort, ULONG ulSize)
{
	for(UINT i = 0; i < m_Sessions.Size(); ++i)
	{
		IDCCSession *pSession = m_Sessions[i];
		_ASSERTE(pSession != NULL);

		String sNick = GetPrefixNick(sPrefix);
		if((pSession->GetType() == DCC_SEND) && pSession->IsIncoming() && (pSession->GetState() == DCC_STATE_WAITING) &&
			(pSession->GetRemotePort() == usPort) && (pSession->GetRemoteUser() == sNick))
		{
			DCCSend* pDCCSend = ((DCCSend*)pSession);
			pDCCSend->Resume();
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
//	IDCCHandler
/////////////////////////////////////////////////////////////////////////////

void DCCHandler::ShowDCCListWindow(bool bShow)
{
	if(bShow)
	{
		if(!m_pDCCListWindow)
		{
			m_pDCCListWindow = new DCCListWindow();
			m_pDCCListWindow->SetMainWindow(m_pMainWindow);
			m_pDCCListWindow->SetDCCHandler(this);
			HRESULT hr = m_pDCCListWindow->Create();
			_ASSERTE(SUCCEEDED(hr));
		}
	}
	else
	{
		if(m_pDCCListWindow)
		{
			DestroyWindow(m_pDCCListWindow->GetWindow());
			m_pDCCListWindow = NULL;
		}
	}
}

UINT DCCHandler::GetSessionCount()
{
	return m_Sessions.Size();
}

IDCCSession* DCCHandler::GetSession(UINT i)
{
	_ASSERTE(i < m_Sessions.Size());

	return m_Sessions[i];
}

void DCCHandler::RemoveSession(IDCCSession* pSession)
{
	_TRACE("DCCHandler(0x%08X)::RemoveSession(0x%08X)", this, pSession);

	UINT index = m_Sessions.Find(pSession);
	if(index != -1)
	{
		if(m_pDCCListWindow)
		{
			m_pDCCListWindow->RemoveSession(pSession);
		}

		delete pSession;
		m_Sessions.Erase(index);
	}
}

void DCCHandler::AddSession(IDCCSession* pSession)
{
	_TRACE("DCCHandler(0x%08X)::AddSession(0x%08X)", this, pSession);
	_ASSERTE(pSession != NULL);

	m_Sessions.Append(pSession);
}

void DCCHandler::Chat(const String& sUser)
{
	_TRACE("DCCHandler(0x%08X)::Chat(\"%s\")", this, sUser.Str());

	DCCListWindow* pDCCWnd = GetDCCListWindow(true);
	_ASSERTE(pDCCWnd != NULL);

	DCCChat *pChat = new DCCChat();
	pChat->SetDCCHandler(this);

	if(pChat->OutgoingRequest(sUser))
	{
		USES_CONVERSION;

		m_Sessions.Append(pChat);
		pDCCWnd->AddSession(pChat);

		USHORT usPort = pChat->GetLocalPort();
		String sAddr = g_Options.GetAddress();

		ULONG ulAddr = inet_addr(T2CA(sAddr.Str()));
		ulAddr = htonl(ulAddr);

		TCHAR buf[100];
		wsprintf(buf, _T("CHAT chat %u %u"), ulAddr, usPort);

		m_pSession->CTCP(sUser, _T("DCC"), buf);
	}
}

void DCCHandler::Send(const String& sUser)
{
	_TRACE("DCCHandler(0x%08X)::Send(\"%s\")", this, sUser.Str());

	DCCListWindow* pDCCWnd = GetDCCListWindow(true);
	_ASSERTE(pDCCWnd != NULL);

	DCCSend *pSend = new DCCSend();
	pSend->SetDCCHandler(this);

	if(pSend->OutgoingRequest(sUser))
	{
		USES_CONVERSION;

		m_Sessions.Append(pSend);
		pDCCWnd->AddSession(pSend);

		USHORT usPort = pSend->GetLocalPort();
		String sAddr = g_Options.GetAddress();

		ULONG ulAddr = inet_addr(T2CA(sAddr.Str()));
		ulAddr = htonl(ulAddr);

		TCHAR buf[POCKETIRC_MAX_IRC_LINE_LEN];
		_sntprintf(buf, sizeof(buf), _T("SEND %s %u %u %u"), pSend->GetFileName().Str(), ulAddr, usPort, pSend->GetFileSize());
		buf[sizeof(buf) - 1] = 0;

		m_pSession->CTCP(sUser, _T("DCC"), buf);
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Internal Methods
/////////////////////////////////////////////////////////////////////////////

DCCListWindow* DCCHandler::GetDCCListWindow(bool bEnsureCreated)
{
	if(bEnsureCreated)
	{
		ShowDCCListWindow(true);
	}
	return m_pDCCListWindow;
}
