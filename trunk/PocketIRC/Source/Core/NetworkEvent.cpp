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

String NetworkEvent::EventIDToString(int idEvent)
{
	String sCmd;

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

int NetworkEvent::EventStringToID(const String& sEvent)
{
	_ASSERTE(sEvent.Str() != NULL);

	int idEvent = IRC_CMD_INVALID;
	if(sEvent.Size())
	{
		const int idUnknown = sizeof(COMMAND_EVENT_STRINGS)/sizeof(COMMAND_EVENT_STRINGS[0]) - 1;
		int idString = GetStringID(COMMAND_EVENT_STRINGS, sEvent.Str(), idUnknown);

		if(idString == idUnknown)
		{
			int idParsed = _ttoi(sEvent.Str());
			if(idParsed > 0)
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

int NetworkEvent::CTCPEventStringToID(const String& sEvent, bool bReply)
{
	_ASSERTE(sEvent.Str() != NULL);

	int idEvent = IRC_CTCP_UNKNOWN;
	if(sEvent.Size())
	{
		const int idUnknown = sizeof(CTCP_EVENT_STRINGS)/sizeof(CTCP_EVENT_STRINGS[0]) - 1;
		int idString = GetStringID(CTCP_EVENT_STRINGS, sEvent.Str(), idUnknown);

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
		String* psParam = va_arg(va, String*);
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

void NetworkEvent::SetEvent(const String& sEvent)
{
	m_sEvent = sEvent;
}

const String& NetworkEvent::GetEvent() const
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


UINT NetworkEvent::AddParam(const String& sParam)
{
	UINT index = m_vecParams.Size();
	m_vecParams[index] = sParam;
	return index;
}

const String& NetworkEvent::GetParam(UINT iParam) const
{
	_ASSERTE(iParam < GetParamCount());
	return m_vecParams[iParam];
}

UINT NetworkEvent::GetParamCount() const
{
	return m_vecParams.Size();
}

void NetworkEvent::SetPrefix(const String& sPrefix)
{
	m_sPrefix = sPrefix;
}

const String& NetworkEvent::GetPrefix() const
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
