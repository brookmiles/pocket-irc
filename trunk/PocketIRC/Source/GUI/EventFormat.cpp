#include "PocketIRC.h"
#include "EventFormat.h"

#include "Config\Options.h"

#include "IrcString.h"

/////////////////////////////////////////////////////////////////////////////
// File private utilities
/////////////////////////////////////////////////////////////////////////////

static int GetEventHighlight(const NetworkEvent& network, const String& sMe)
{
	switch(network.GetEventID())
	{
		case IRC_CMD_PRIVMSG:
		case IRC_CMD_NOTICE:
		case IRC_CTCP_ACTION:
			if(_tcsstr(network.GetParam(1).Str(), sMe.Str()))
				return DisplayEvent::HL_NOTIFY;
			else
				return DisplayEvent::HL_MESSAGE;

		default:
			return DisplayEvent::HL_MISC;
	}
}

static String GetTimestamp()
{
	String str;
	str.Reserve(10);

	SYSTEMTIME st;
	GetLocalTime(&st);

	wsprintf(str.Str(), _T("[%02u:%02u] "), st.wHour, st.wMinute);

	return str;
}

static void GetEventFormat(const NetworkEvent& network, const String& sMe, 
	const String& sFmt, String& sText, int* piHighlight)
{
	sText.Reserve(POCKETIRC_MAX_IRC_DISPLAY_LEN + 1);

	*piHighlight = GetEventHighlight(network, sMe);

	unsigned nFmtLen = sFmt.Size();
	UINT nUsed = 0;

	String str;
	str.Reserve(POCKETIRC_MAX_IRC_DISPLAY_LEN + 1);

	if(g_Options.GetShowTimestamp())
	{
		sText += GetTimestamp();
		nUsed = sText.Size();
	}

	UINT i = 0;
	while(i < nFmtLen && nUsed < POCKETIRC_MAX_IRC_DISPLAY_LEN)
	{
		if(sFmt[i] == POCKETIRC_FORMAT_CHAR)
		{
			i++;
			str = _T("");

			TCHAR c = sFmt[i++];
			switch(c)
			{
				case POCKETIRC_FORMAT_CHAR:
					str += POCKETIRC_FORMAT_CHAR;
				break;
				case 'u':
					str = sMe;
				break;
				case 'n':
					str = GetPrefixNick(network.GetPrefix());
				break;
				case 'h':
					str = GetPrefixHost(network.GetPrefix());
				break;
				case 'i':
					str = GetPrefixIdent(network.GetPrefix());
				break;
				case 't':
					str = GetEventKey(network);
				break;
				case 'c':
					if(network.GetEvent().Size() > 0)
					{
						str = network.GetEvent();
					}
					else
					{
						str = NetworkEvent::EventIDToString(network.GetEventID());
					}
				break;
				case ' ':
					// Ignore and advance, indended for breaking up a sequence
					// "%0% - Hi" Prints "FIRST- Hi" instead of "FIRST REST Hi"
				break;
				default:
				{
					if((c >= '0' && c <= '9') || c == '*')
					{
						UINT nParam = (c == '*') ? 0 : (c - '0');
						bool bAll = (c == '*') ? true : ((sFmt[i] == '-') ? (i++, true) : false);

						UINT nStrUsed = 0;
						for(UINT j = nParam; (j < network.GetParamCount()) && (nStrUsed < POCKETIRC_MAX_IRC_DISPLAY_LEN); ++j)
						{
							const String& sParam = network.GetParam(j);
							unsigned nParamLen = sParam.Size();

							if(j > nParam) nParamLen++;
							if(nStrUsed + nParamLen < POCKETIRC_MAX_IRC_DISPLAY_LEN)
							{
								if(j > nParam) str += ' ';
								str += sParam;

								nStrUsed += nParamLen;
							}
							else
							{
								break;
							}

							if(!bAll)
							{
								break;
							}
						}
					}
					else
					{
						// Ignore errant $ escape character
						i--;
					}
				}
				break;
			}

			unsigned nStrLen = str.Size();
			if(nStrLen)
			{
				if(nUsed + nStrLen <= POCKETIRC_MAX_IRC_DISPLAY_LEN)
				{
					sText += str;
					nUsed += nStrLen;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			sText += sFmt[i++];
			nUsed++;
		}
	}

	if(_tcsstr(sText.Str(), sMe.Str()))
	{
		*piHighlight = DisplayEvent::HL_NOTIFY;
	}
}

String GetEventKey(const NetworkEvent& network)
{
	int idEvent = network.GetEventID();
	switch(idEvent)
	{
		case IRC_CMD_PRIVMSG:
		case IRC_CMD_NOTICE:
		case IRC_CTCP_ACTION:
			if(IsChannel(network.GetParam(0)))
			{
				return network.GetParam(0);
			}
			else
			{
				if(network.IsIncoming())
				{
					return GetPrefixNick(network.GetPrefix());
				}
				else
				{
					return network.GetParam(0);
				}
			}
		case IRC_CMD_JOIN:
		case IRC_CMD_PART:
		case IRC_CMD_KICK:
		case IRC_CMD_MODE:
		case IRC_CMD_TOPIC:
			return network.GetParam(0);
		case IRC_RPL_TOPIC:
		case IRC_RPL_ENDOFNAMES:
		case IRC_ERR_CANNOTSENDTOCHAN: //"<channel name> :Cannot send to channel"
		case IRC_ERR_NOTONCHANNEL: //"<channel> :You're not on that channel"
		case IRC_ERR_KEYSET: //"<channel> :Channel key already set"
		case IRC_ERR_CHANNELISFULL: //"<channel> :Cannot join channel (+l)"
		case IRC_ERR_INVITEONLYCHAN: //"<channel> :Cannot join channel (+i)"
		case IRC_ERR_BANNEDFROMCHAN: //"<channel> :Cannot join channel (+b)"
		case IRC_ERR_BADCHANNELKEY: //"<channel> :Cannot join channel (+k)"
		case IRC_ERR_BADCHANMASK: //"<channel> :Bad Channel Mask"
		case IRC_ERR_NOCHANMODES: //"<channel> :Channel doesn't support modes"
		case IRC_ERR_BANLISTFULL: //"<channel> <char> :Channel list is full"
		case IRC_ERR_CHANOPRIVSNEEDED: //"<channel> :You're not channel operator"
			return network.GetParam(1);
		case IRC_RPL_NAMREPLY:
		case IRC_ERR_USERNOTINCHANNEL: //"<nick> <channel> :They aren't on that channel"
		case IRC_ERR_USERONCHANNEL: //"<user> <channel> :is already on channel"
			return network.GetParam(2);//wtf?
		default:
			return _T("");
	}
}

/////////////////////////////////////////////////////////////////////////////
// Internal Methods
/////////////////////////////////////////////////////////////////////////////

String EventFormat(const NetworkEvent& network, const String& sMe, const String& sFmt)
{
	int iHighlight;
	String sText;

	GetEventFormat(network, sMe, sFmt, sText, &iHighlight);

	return sText;
}

void DisplayEventFormat(DisplayEvent& display, const NetworkEvent& network, const String& sMe)
{
	display.SetKey(GetEventKey(network));

	int iHighlight;
	String sText;

	String sFmt = g_Options.GetEventFormat(network.GetEventID(), network.IsIncoming());
	GetEventFormat(network, sMe, sFmt, sText, &iHighlight);

	display.SetText(sText);
	display.SetHighlight(iHighlight);
}
