#include "PocketIRC.h"
#include "Config/Options.h"
#include "Writer.h"
#include "StringConv.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
/////////////////////////////////////////////////////////////////////////////

Writer::Writer()
{
	_TRACE("Writer(0x%08X)::Writer()", this);

	m_pTransport = NULL;
}

Writer::~Writer()
{
	_TRACE("Writer(0x%08X)::~Writer()", this);

}

/////////////////////////////////////////////////////////////////////////////
// Interface
/////////////////////////////////////////////////////////////////////////////

void Writer::SetTransport(ITransportWrite* pTransport)
{
	_TRACE("Writer(0x%08X)::SetTransport(0x%08X)", this, pTransport);

	m_pTransport = pTransport;
}

/////////////////////////////////////////////////////////////////////////////
// Messages
/////////////////////////////////////////////////////////////////////////////

void Writer::Write(const tstring& sMsg)
{
	_ASSERTE(sMsg.c_str() != NULL);
	_TRACE("Writer(0x%08X)::Raw(\"%s\")", this, sMsg.c_str());

	// TODO really shouldn't be reading global options from here :/
	std::string msgWithReturn = g_Options.GetUTF8() ? TCHARToUTF8(sMsg) : TCHARToMB(sMsg);
	msgWithReturn.append("\r\n");

	m_pTransport->Write((BYTE*)msgWithReturn.c_str(), msgWithReturn.size());
}

void Writer::WriteEvent(const NetworkEvent& event)
{
	_TRACE("Writer(0x%08X)::WriteEvent(%d)", this, event.GetEventID());

	const int idEvent = event.GetEventID();
	TCHAR buf[POCKETIRC_MAX_IRC_LINE_LEN + 1];

	tstring sCmd = event.GetEvent();
	if(!sCmd.size())
	{
		sCmd = NetworkEvent::EventIDToString(idEvent);
		_ASSERTE(sCmd.size());
	}

	if(idEvent >= EVENT_OFFSET_COMMAND && idEvent <= IRC_CMD_UNKNOWN)
	{
		// <command> <parameter[0 to (n-1)]> :<parameter[n]>

		int nUsed = _sntprintf(buf, POCKETIRC_MAX_IRC_LINE_LEN, _T("%s"), sCmd.c_str());

		for(UINT i = 0; (i < event.GetParamCount()) && (nUsed < POCKETIRC_MAX_IRC_LINE_LEN); ++i)
		{
			const tstring& sParam = event.GetParam(i);
			_ASSERTE(sParam.c_str() != NULL);

			if(i < event.GetParamCount() - 1 || !event.GetAutoPrefix())
			{
				nUsed += _sntprintf(buf + nUsed, POCKETIRC_MAX_IRC_LINE_LEN - nUsed, _T(" %s"), sParam.c_str());
			}
			else
			{
				// prefix last parameter with ':'
				nUsed += _sntprintf(buf + nUsed, POCKETIRC_MAX_IRC_LINE_LEN - nUsed, _T(" :%s"), sParam.c_str());
			}
		}

		// If the line length is == POCKETIRC_MAX_IRC_LINE_LEN
		// then we need to add a null at the end, sntprintf won't
		buf[nUsed] = '\0';

		Write(buf);
	}
	else if(idEvent >= EVENT_OFFSET_CTCP && idEvent <= IRC_CTCP_UNKNOWN || 
		idEvent >= EVENT_OFFSET_CTCP_REPLY && idEvent <= IRC_CTCP_RPL_UNKNOWN)
	{
		if(event.GetParamCount() < 1)
		{
			_TRACE("Writer(0x%08X)::WriteEvent() CTCP TO NOBODY NetworkEvent(idEvent = %d)", this, idEvent);
		}
		else
		{
			// PRIVMSG <parameter[0]> :\x01<CTCP command> <parameter[1 to n]\x01

			int nUsed = 0;
			if(idEvent >= EVENT_OFFSET_CTCP_REPLY && idEvent <= IRC_CTCP_RPL_UNKNOWN)
			{
				nUsed += _sntprintf(buf, POCKETIRC_MAX_IRC_LINE_LEN, _T("NOTICE %s :\x01%s"), 
					event.GetParam(0).c_str(), sCmd.c_str());
			}
			else
			{
				nUsed += _sntprintf(buf, POCKETIRC_MAX_IRC_LINE_LEN, _T("PRIVMSG %s :\x01%s"), 
					event.GetParam(0).c_str(), sCmd.c_str());
			}

			for(UINT i = 1; (i < event.GetParamCount()) && (nUsed < POCKETIRC_MAX_IRC_LINE_LEN - 1); ++i)
			{
				const tstring& sParam = event.GetParam(i);
				_ASSERTE(sParam.c_str() != NULL);

				nUsed += _sntprintf(buf + nUsed, POCKETIRC_MAX_IRC_LINE_LEN - nUsed - 1, _T(" %s"), sParam.c_str());
			}

			// Add CTCP terminator, nUsed will be < POCKETIRC_MAX_IRC_LINE_LEN
			nUsed += _sntprintf(buf + nUsed, POCKETIRC_MAX_IRC_LINE_LEN - nUsed, _T("\x01"));

			// If the line length is == POCKETIRC_MAX_IRC_LINE_LEN
			// then we need to add a null at the end, sntprintf won't
			buf[nUsed] = '\0';

			Write(buf);
		}
	}
	else
	{
		_TRACE("Writer(0x%08X)::WriteEvent() GOT UNKNOWN NetworkEvent(idEvent = %d)", this, idEvent);
		_ASSERTE(FALSE);
	}
}
