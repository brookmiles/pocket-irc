#include "PocketIRC.h"
#include "Reader.h"

#include "Network\SocketTransport.h"
#include "Config\Options.h"

#include "IrcString.h"
#include "IrcStringFormat.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

Reader::Reader()
{
	_TRACE("Reader(0x%08X)::Reader()", this);

	m_pNotify = NULL;
}


Reader::~Reader()
{
	_TRACE("Reader(0x%08X)::~Reader()", this);
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

void Reader::SetNotifySink(INetworkEventNotify* pNotify)
{
	_TRACE("Reader(0x%08X)::SetNotifySink(0x%08X)", this, pNotify);
	m_pNotify = pNotify;
}

/////////////////////////////////////////////////////////////////////////////
//	ITransportReader Interface
/////////////////////////////////////////////////////////////////////////////

void Reader::OnConnect(HRESULT hr, LPCTSTR pszError)
{
	_TRACE("Reader(0x%08X)::OnConnect(0x%08X, \"%s\")", this, hr, pszError);

	NetworkEvent event;

	if(FAILED(hr))
	{
		TCHAR buf[200] = _T("");
		_sntprintf(buf, sizeof(buf)/sizeof(buf[0]), _T("%s (0x%08X)"), SocketTransport::GetWSAErrorMessage(HRESULT_CODE(hr)), hr);

		event.SetEventID(SYS_EVENT_CONNECTFAILED);
		event.AddParam(buf);
	}
	else
	{
		if(hr == S_OK)
		{
			event.SetEventID(SYS_EVENT_CONNECTED);
			event.AddParam(pszError);
		}
		else
		{
			event.SetEventID(SYS_EVENT_TRYCONNECT);
			event.AddParam(pszError);
		}
	}

	m_pNotify->OnEvent(event);
}

void Reader::OnError(HRESULT hr)
{
	_TRACE("Reader(0x%08X)::OnError(0x%08X)", this, hr);

	TCHAR buf[200] = _T("");
	_sntprintf(buf, sizeof(buf)/sizeof(buf[0]), _T("%s (0x%08X)"), SocketTransport::GetWSAErrorMessage(HRESULT_CODE(hr)), hr);

	NetworkEvent event(SYS_EVENT_ERROR, 1, &String(buf));
	m_pNotify->OnEvent(event);
}

void Reader::OnClose()
{
	_TRACE("Reader(0x%08X)::OnClose()", this);

	NetworkEvent event(SYS_EVENT_CLOSE, 0);
	m_pNotify->OnEvent(event);
}

void Reader::OnLineRead(LPCTSTR pszLine)
{
	_TRACE("Reader(0x%08X)::OnLineRead(\"%s\")", this, pszLine);

	NetworkEvent event;
	event.SetIncoming(true);

	String sStripped;
	if(g_Options.GetStripIncoming())
	{
		sStripped = StringFormat::StripFormatting(pszLine);
		pszLine = sStripped.Str();
	}

	bool bParseOk = ParseEvent(event, pszLine);
	_ASSERTE(bParseOk);
	if(!bParseOk)
	{
		_TRACE("PARSE FAILED: \"%s\"", pszLine);
	}
	else
	{
		m_pNotify->OnEvent(event);
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Static Utility Methods
/////////////////////////////////////////////////////////////////////////////

bool Reader::ParseEvent(NetworkEvent& event, LPCTSTR pszInput)
{
	_ASSERTE(pszInput != NULL);
	
	TCHAR buf[POCKETIRC_MAX_IRC_LINE_LEN + 1];
	LPCTSTR seek = pszInput;
	LPCTSTR tmp;
	
	// Retrieve first word into buffer
	// This is either the prefix or the command
	if(IrcStringGetWord(buf, seek, sizeof(buf)/sizeof(TCHAR), &tmp) == 0)
		return false;

	if(buf[0] == (TCHAR)':')
	{
		// We have a prefix
		event.SetPrefix(buf + 1);

		// Seek ahead to command
		seek = IrcStringSkipSpaces(tmp);
	}

	// Get next word, this should be the command
	if(IrcStringGetWord(buf, seek, sizeof(buf)/sizeof(TCHAR), &tmp) == 0)
		return false;
	seek = IrcStringSkipSpaces(tmp);

	int idEvent = NetworkEvent::EventStringToID(buf);
	event.SetEventID(idEvent);
	event.SetEvent(buf);

	// Get parameters, if any
	while(IrcStringGetWord(buf, seek, sizeof(buf)/sizeof(TCHAR), &tmp) != 0)
	{
		if(buf[0] == (TCHAR)':')
		{
			// If it's a PRIVMSG, check to see if it's CTCP, and if so
			// change the command id as well as splitting up the CTCP
			// parameters
			//
			// ps. replies to these commands come via NOTICE.  argh.
			if((idEvent == IRC_CMD_PRIVMSG || idEvent == IRC_CMD_NOTICE) && buf[1] == (TCHAR)'\x01')
			{
				bool bReply = (idEvent == IRC_CMD_NOTICE);
				// Extract CTCP command
				IrcStringGetWord(buf, seek + 2, sizeof(buf)/sizeof(TCHAR), &tmp);
				seek = IrcStringSkipSpaces(tmp);

				int len = _tcslen(buf);
				_ASSERTE(len > 0);
				if(len < 1)
					return false;

				if(buf[len - 1] == '\x01')
				{
					buf[len - 1] = '\0';
					seek--;
				}

				// Copy CTCP command
				idEvent = NetworkEvent::CTCPEventStringToID(buf, bReply);
				event.SetEventID(idEvent);
				event.SetEvent(buf);

				// Copy CTCP parameters into buf, this MAY include a terminating \1
				_tcsncpy(buf, seek, sizeof(buf)/sizeof(TCHAR));
				len = _tcslen(buf);

				// Replace \1 terminator with NULL
				if(len > 0 && buf[len - 1] == '\x01')
				{
					buf[len - 1] = '\0';
					len--;
				}
				else
				{
					_TRACE("Reader::ParseEvent() CTCP MISSING TERMINATOR");
				}

				if(len > 0)
				{
					// Copy final CTCP parameters
					event.AddParam(buf);
				}
			}
			else
			{
				if(idEvent == IRC_CMD_PRIVMSG && g_Options.PresButan())
				{
					int r = rand() % 10;

					if(r == 1)
					{
						event.AddParam(_T("pres butan"));
					}
					else if(r == 2)
					{
						event.AddParam(_T("role dise"));
					}
					else if(r == 3)
					{
						event.AddParam(_T("pres butan to go"));
					}
					else if(r == 4)
					{
						event.AddParam(_T("huhu"));
					}
					else if(r == 5)
					{
						event.AddParam(_T("no u"));
					}
					else
					{
						event.AddParam(seek + 1);
					}
				}
				else
				{
					// Parameter is the entire rest of the line
					// Copy from seek because buf only contains a single word
					event.AddParam(seek + 1);
				}
			}
			break;
		}
		else
		{
			// Get next paramter
			event.AddParam(buf);
			seek = IrcStringSkipSpaces(tmp);
		}
	}

	// Special case, NOTICE AUTH at login.
	if(idEvent == IRC_CMD_NOTICE && event.GetPrefix().Size() == 0 && 
		event.GetParamCount() > 0)
	{
		event.SetPrefix(event.GetParam(0));
	}

	return true;
}
