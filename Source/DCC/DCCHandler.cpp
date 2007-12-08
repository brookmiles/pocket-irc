#include "PocketIRC.h"
#include "DCCHandler.h"

#include "IrcString.h"
#include "StringUtil.h"

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

		const tstring& sParams = event.GetParam(1);
		tstring sCmd = GetWord(sParams, 0);

		if(Compare(sCmd, _T("SEND"), false))
		{
			if(!g_Options.GetIgnoreSend())
			{
				tstring sFileName = GetWord(sParams, 1);
				ULONG ulAddr = _tcstoul(GetWord(sParams, 2).c_str(), NULL, 10);
				USHORT usPort = (USHORT)_tcstoul(GetWord(sParams, 3).c_str(), NULL, 10);
				ULONG ulSize = _tcstoul(GetWord(sParams, 4).c_str(), NULL, 10);

				TCHAR* pszFile = _tcsrchr(sFileName.c_str(), '/');
				if(pszFile == NULL)
					 pszFile = _tcsrchr(sFileName.c_str(), '\\');
				if(pszFile != NULL && pszFile[0] != NULL)
				{
					UINT nFileName = pszFile - sFileName.c_str() + 1;
					sFileName = sFileName.substr(nFileName + 1);
				}

				_ASSERTE(usPort != 0);
				_ASSERTE(ulAddr != 0 && ulAddr != -1);

				if(ulAddr == 0 || usPort == 0)
				{
					_TRACE("...INVALID ADDR OR PORT (%s):(%s)", GetWord(sParams, 3).c_str(), GetWord(sParams, 4).c_str());
				}
				else if(sFileName.size() == 0)
				{
					_TRACE("...INVALID FILENAME");
				}
				else
				{
					OnSend(event.GetPrefix(), sFileName, ulAddr, usPort, ulSize);
				}
			}
		}
		else if(Compare(sCmd, _T("CHAT"), false))
		{
			if(!g_Options.GetIgnoreChat())
			{
				tstring sType = GetWord(sParams, 1);
				ULONG ulAddr = _tcstoul(GetWord(sParams, 2).c_str(), NULL, 10);
				USHORT usPort = (USHORT)_tcstoul(GetWord(sParams, 3).c_str(), NULL, 10);

				_ASSERTE(usPort != 0);
				_ASSERTE(ulAddr != 0 && ulAddr != -1);

				if(ulAddr == 0 || usPort == 0)
				{
					_TRACE("...INVALID ADDR OR PORT (%s):(%s)", GetWord(sParams, 3).c_str(), GetWord(sParams, 4).c_str());
				}
				else if(!Compare(sType, _T("CHAT"), false))
				{
					_TRACE("...INVALID CHAT TYPE (%s)", sType.c_str());
				}
				else
				{
					OnChat(event.GetPrefix(), sType, ulAddr, usPort);
				}
			}
		}
		else if(Compare(sCmd, _T("RESUME"), false) || Compare(sCmd, _T("ACCEPT"), false))
		{
			if(m_Sessions.size())
			{
				tstring sFileName = GetWord(sParams, 1);
				USHORT usPort = (USHORT)_tcstoul(GetWord(sParams, 2).c_str(), NULL, 10);
				ULONG ulSize = _tcstoul(GetWord(sParams, 3).c_str(), NULL, 10);

				TCHAR* pszFile = _tcsrchr(sFileName.c_str(), '/');
				if(pszFile == NULL)
					 pszFile = _tcsrchr(sFileName.c_str(), '\\');
				if(pszFile != NULL && pszFile[0] != NULL)
				{
					UINT nFileName = pszFile - sFileName.c_str() + 1;
					sFileName = sFileName.substr(nFileName + 1);
				}

				_ASSERTE(usPort != 0);

				if(usPort == 0)
				{
					_TRACE("...INVALID PORT (%s)", GetWord(sParams, 3).c_str());
				}
				else if(sFileName.size() == 0)
				{
					_TRACE("...INVALID FILENAME");
				}
				else
				{
					if(Compare(sCmd, _T("RESUME"), false))
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
			_TRACE("...UNKNOWN COMMAND (%s)", sCmd.c_str());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Command Handlers
/////////////////////////////////////////////////////////////////////////////

bool DCCHandler::OnSend(const tstring& sPrefix, const tstring& sFileName, ULONG ulAddr, USHORT usPort, ULONG ulSize)
{
	_TRACE("DCCHandler(0x%08X)::OnSend(\"%s\", \r\n\t\"%s\", 0x%08X, %u, %u)", this, sPrefix.c_str(), sFileName.c_str(), ulAddr, usPort, ulSize);

	DCCListWindow* pDCCWnd = GetDCCListWindow(true);
	_ASSERTE(pDCCWnd != NULL);

	DCCSend *pSend = new DCCSend();
	pSend->SetDCCHandler(this);
	pSend->IncomingRequest(sPrefix, ulAddr, usPort, sFileName, ulSize);

	m_Sessions.push_back(pSend);

	pDCCWnd->AddSession(pSend);

	return true;
}

bool DCCHandler::OnChat(const tstring& sPrefix, const tstring& sType, ULONG ulAddr, USHORT usPort)
{
	_TRACE("DCCHandler(0x%08X)::OnChat(\"%s\", \r\n\t\"%s\", 0x%08X, %u)", this, sPrefix.c_str(), sType.c_str(), ulAddr, usPort);

	DCCListWindow* pDCCWnd = GetDCCListWindow(true);
	_ASSERTE(pDCCWnd != NULL);

	DCCChat *pChat = new DCCChat();
	pChat->SetDCCHandler(this);
	pChat->IncomingRequest(sPrefix, ulAddr, usPort);

	m_Sessions.push_back(pChat);

	pDCCWnd->AddSession(pChat);

	return true;
}

bool DCCHandler::OnResume(const tstring& sPrefix, const tstring& sFileName, USHORT usPort, ULONG ulSize)
{
	for(UINT i = 0; i < m_Sessions.size(); ++i)
	{
		IDCCSession *pSession = m_Sessions[i];
		_ASSERTE(pSession != NULL);

		tstring sNick = GetPrefixNick(sPrefix);
		if((pSession->GetType() == DCC_SEND) && !pSession->IsIncoming() && (pSession->GetState() == DCC_STATE_WAITING) && 
			(pSession->GetLocalPort() == usPort) && (pSession->GetRemoteUser() == sNick))
		{
			DCCSend* pDCCSend = ((DCCSend*)pSession);
			if(pDCCSend->Resume(ulSize))
			{
				TCHAR buf[POCKETIRC_MAX_IRC_LINE_LEN];
				_sntprintf(buf, sizeof(buf), _T("ACCEPT %s %u %u"), sFileName.c_str(), usPort, ulSize);
				buf[sizeof(buf) - 1] = 0;

				m_pSession->CTCP(sNick, _T("DCC"), buf);
			}
		}
	}
	return true;
}

bool DCCHandler::OnAccept(const tstring& sPrefix, const tstring& sFileName, USHORT usPort, ULONG ulSize)
{
	for(UINT i = 0; i < m_Sessions.size(); ++i)
	{
		IDCCSession *pSession = m_Sessions[i];
		_ASSERTE(pSession != NULL);

		tstring sNick = GetPrefixNick(sPrefix);
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
	return m_Sessions.size();
}

IDCCSession* DCCHandler::GetSession(UINT i)
{
	_ASSERTE(i < m_Sessions.size());

	return m_Sessions[i];
}

void DCCHandler::RemoveSession(IDCCSession* pSession)
{
	_TRACE("DCCHandler(0x%08X)::RemoveSession(0x%08X)", this, pSession);

	std::vector<IDCCSession*>::iterator i = std::find(m_Sessions.begin(), m_Sessions.end(), pSession);
	if(i != m_Sessions.end())
	{
		if(m_pDCCListWindow)
		{
			m_pDCCListWindow->RemoveSession(*i);
		}

		delete *i;
		m_Sessions.erase(i);
	}
}

void DCCHandler::AddSession(IDCCSession* pSession)
{
	_TRACE("DCCHandler(0x%08X)::AddSession(0x%08X)", this, pSession);
	_ASSERTE(pSession != NULL);

	m_Sessions.push_back(pSession);
}

void DCCHandler::Chat(const tstring& sUser)
{
	_TRACE("DCCHandler(0x%08X)::Chat(\"%s\")", this, sUser.c_str());

	DCCListWindow* pDCCWnd = GetDCCListWindow(true);
	_ASSERTE(pDCCWnd != NULL);

	DCCChat *pChat = new DCCChat();
	pChat->SetDCCHandler(this);

	if(pChat->OutgoingRequest(sUser))
	{
		USES_CONVERSION;

		m_Sessions.push_back(pChat);
		pDCCWnd->AddSession(pChat);

		USHORT usPort = pChat->GetLocalPort();
		tstring sAddr = g_Options.GetAddress();

		ULONG ulAddr = inet_addr(T2CA(sAddr.c_str()));
		ulAddr = htonl(ulAddr);

		TCHAR buf[100];
		wsprintf(buf, _T("CHAT chat %u %u"), ulAddr, usPort);

		m_pSession->CTCP(sUser, _T("DCC"), buf);
	}
}

void DCCHandler::Send(const tstring& sUser)
{
	_TRACE("DCCHandler(0x%08X)::Send(\"%s\")", this, sUser.c_str());

	DCCListWindow* pDCCWnd = GetDCCListWindow(true);
	_ASSERTE(pDCCWnd != NULL);

	DCCSend *pSend = new DCCSend();
	pSend->SetDCCHandler(this);

	if(pSend->OutgoingRequest(sUser))
	{
		USES_CONVERSION;

		m_Sessions.push_back(pSend);
		pDCCWnd->AddSession(pSend);

		USHORT usPort = pSend->GetLocalPort();
		tstring sAddr = g_Options.GetAddress();

		ULONG ulAddr = inet_addr(T2CA(sAddr.c_str()));
		ulAddr = htonl(ulAddr);

		TCHAR buf[POCKETIRC_MAX_IRC_LINE_LEN];
		_sntprintf(buf, sizeof(buf), _T("SEND %s %u %u %u"), pSend->GetFileName().c_str(), ulAddr, usPort, pSend->GetFileSize());
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
