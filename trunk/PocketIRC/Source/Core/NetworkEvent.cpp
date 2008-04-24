#include "PocketIRC.h"
#include "NetworkEvent.h"

/////////////////////////////////////////////////////////////////////////////
// Known Command Tables
/////////////////////////////////////////////////////////////////////////////

LPCTSTR COMMAND_EVENT_STRINGS[] = {
	_T("NICK"),
	_T("USER"),
	_T("PASS"),
	_T("JOIN"),
	_T("PART"),
	_T("KICK"),
	_T("INVITE"),
	_T("PRIVMSG"),
	_T("NOTICE"),
	_T("MODE"),
	_T("PING"),
	_T("PONG"),
	_T("TOPIC"),
	_T("WHOIS"),
	_T("WHOWAS"),
	_T("NAMES"),
	_T("LIST"),
	_T("USERHOST"),
	_T("QUIT"),
	_T("AWAY"),
	_T("ERROR"),
	_T("UNKNOWN"),
};

LPCTSTR CTCP_EVENT_STRINGS[] = {
	_T("ACTION"),
	_T("PING"),
	_T("VERSION"),
	_T("DCC"),
	_T("UNKNOWN"),
};

/////////////////////////////////////////////////////////////////////////////
// File Private Utilities
/////////////////////////////////////////////////////////////////////////////

static int GetStringID(LPCTSTR* ppszList, LPCTSTR psz, int iLast)
{
	_ASSERTE(ppszList != NULL);

	for(int i = 0; i < iLast; ++i)
	{
		_ASSERTE(ppszList[i] != NULL);

		if(_tcsicmp(psz, ppszList[i]) == 0)
			return i;
	}
	return iLast;
}

/////////////////////////////////////////////////////////////////////////////
// Static Utilities
/////////////////////////////////////////////////////////////////////////////

tstring NetworkEvent::EventIDToString(int idEvent)
{
	tstring sCmd;

	if(idEvent >= EVENT_OFFSET_COMMAND && idEvent <= IRC_CMD_UNKNOWN)
	{
		sCmd = COMMAND_EVENT_STRINGS[idEvent - EVENT_OFFSET_COMMAND];
	}
	else if(idEvent >= EVENT_OFFSET_CTCP && idEvent <= IRC_CTCP_UNKNOWN)
	{
		sCmd = CTCP_EVENT_STRINGS[idEvent - EVENT_OFFSET_CTCP];
	}
	else if(idEvent >= EVENT_OFFSET_CTCP_REPLY && idEvent <= IRC_CTCP_RPL_UNKNOWN)
	{
		sCmd = CTCP_EVENT_STRINGS[idEvent - EVENT_OFFSET_CTCP_REPLY];
	}
	else if(idEvent > 0)
	{
		TCHAR buf[20];
		wsprintf(buf, _T("%03d"), idEvent);

		sCmd = buf;
	}
	return sCmd;
}

int NetworkEvent::EventStringToID(const tstring& sEvent)
{
	_ASSERTE(sEvent.c_str() != NULL);

	int idEvent = IRC_CMD_INVALID;
	if(sEvent.size())
	{
		const int idUnknown = sizeof(COMMAND_EVENT_STRINGS)/sizeof(COMMAND_EVENT_STRINGS[0]) - 1;
		int idString = GetStringID(COMMAND_EVENT_STRINGS, sEvent.c_str(), idUnknown);

		if(idString == idUnknown)
		{
			TCHAR* end = 0;
			int idParsed = _tcstol(sEvent.c_str(), &end, 10);
			if(*end == 0 && idParsed > 0)
			{
				idEvent = idParsed;
			}
			else
			{
				idEvent = IRC_CMD_UNKNOWN;
			}
		}
		else
		{
			idEvent = idString + EVENT_OFFSET_COMMAND;
		}
	}
	return idEvent;
}

int NetworkEvent::CTCPEventStringToID(const tstring& sEvent, bool bReply)
{
	_ASSERTE(sEvent.c_str() != NULL);

	int idEvent = IRC_CTCP_UNKNOWN;
	if(sEvent.size())
	{
		const int idUnknown = sizeof(CTCP_EVENT_STRINGS)/sizeof(CTCP_EVENT_STRINGS[0]) - 1;
		int idString = GetStringID(CTCP_EVENT_STRINGS, sEvent.c_str(), idUnknown);

		if(bReply)
		{
			idEvent = idString + EVENT_OFFSET_CTCP_REPLY;
		}
		else
		{
			idEvent = idString + EVENT_OFFSET_CTCP;
		}
	}
	return idEvent;
}

bool NetworkEvent::IsReplyEvent(int idEvent)
{
	return (idEvent > 0) && (idEvent < EVENT_OFFSET_COMMAND);
}

bool NetworkEvent::IsCTCPEvent(int idEvent)
{
	return (idEvent >= EVENT_OFFSET_CTCP) && (idEvent <= IRC_CTCP_UNKNOWN);
}

bool NetworkEvent::IsCTCPReplyEvent(int idEvent)
{
	return (idEvent >= EVENT_OFFSET_CTCP_REPLY) && (idEvent <= IRC_CTCP_RPL_UNKNOWN);
}

bool NetworkEvent::IsCommandEvent(int idEvent)
{
	return (idEvent >= EVENT_OFFSET_COMMAND) && (idEvent <= IRC_CMD_UNKNOWN);
}

bool NetworkEvent::IsSystemEvent(int idEvent)
{
	return (idEvent >= EVENT_OFFSET_SYSTEM) && (idEvent <= SYS_EVENT_UNKNOWN);
}


/////////////////////////////////////////////////////////////////////////////
// Constructors/Destructors
/////////////////////////////////////////////////////////////////////////////

NetworkEvent::NetworkEvent()
{
	m_idEvent = 0;
	m_bIncoming = false;
	m_bAutoPrefix = true;
}

NetworkEvent::NetworkEvent(int idEvent, int nParams, ...)
{
	m_idEvent = idEvent;
	m_bIncoming = false;
	m_bAutoPrefix = true;

	va_list va;
	va_start(va, nParams);

	for(int i = 0; i < nParams; ++i)
	{
		tstring* psParam = va_arg(va, tstring*);
		_ASSERTE(psParam != NULL);

		AddParam(*psParam);
	}
	va_end(va);
}

NetworkEvent::~NetworkEvent()
{

}


/////////////////////////////////////////////////////////////////////////////
// Interface
/////////////////////////////////////////////////////////////////////////////

void NetworkEvent::SetEvent(const tstring& sEvent)
{
	m_sEvent = sEvent;
}

const tstring& NetworkEvent::GetEvent() const
{
	return m_sEvent;
}


void NetworkEvent::SetEventID(int idEvent)
{
	m_idEvent = idEvent;
}

int NetworkEvent::GetEventID() const
{
	return m_idEvent;
}


UINT NetworkEvent::AddParam(const tstring& sParam)
{
	m_vecParams.push_back(sParam);
	return GetParamCount();
}

const tstring& NetworkEvent::GetParam(UINT iParam) const
{
	_ASSERTE(iParam < GetParamCount());
	if(iParam < GetParamCount())
	{
		return m_vecParams[iParam];
	}

	static const tstring empty;
	return empty;
}

UINT NetworkEvent::GetParamCount() const
{
	return m_vecParams.size();
}

void NetworkEvent::SetPrefix(const tstring& sPrefix)
{
	m_sPrefix = sPrefix;
}

const tstring& NetworkEvent::GetPrefix() const
{
	return m_sPrefix;
}

void NetworkEvent::SetIncoming(bool bIncoming)
{
	m_bIncoming = bIncoming;
}

bool NetworkEvent::IsIncoming() const
{
	return m_bIncoming;
}

void NetworkEvent::SetAutoPrefix(bool bAutoPrefix)
{
	m_bAutoPrefix = bAutoPrefix;
}

bool NetworkEvent::GetAutoPrefix() const
{
	return m_bAutoPrefix;
}
