#ifndef _NETWORKEVENT_H_INCLUDED_
#define _NETWORKEVENT_H_INCLUDED_

#include "Vector.h"
#include "StringT.h"

#define EVENT_OFFSET_COMMAND  1000
#define EVENT_OFFSET_CTCP 2000
#define EVENT_OFFSET_CTCP_REPLY 2500
#define EVENT_OFFSET_SYSTEM 3000

class NetworkEvent
{
public:
	static String EventIDToString(int idEvent);
	static int EventStringToID(const String& sEvent);
	static int CTCPEventStringToID(const String& sEvent, bool bReply = false);

	static bool IsReplyEvent(int idEvent);
	static bool IsCommandEvent(int idEvent);
	static bool IsCTCPEvent(int idEvent);
	static bool IsCTCPReplyEvent(int idEvent);
	static bool IsSystemEvent(int idEvent);

	NetworkEvent();
	NetworkEvent(int idEvent, int nParams, ...);
	~NetworkEvent();

	void SetEvent(const String& sEvent);
	const String& GetEvent() const;

	void SetEventID(int idEvent);
	int GetEventID() const;

	UINT AddParam(const String& sParam);
	const String& GetParam(UINT iParam) const;
	UINT GetParamCount() const;

	void SetPrefix(const String& sPrefix);
	const String& GetPrefix() const;

	bool IsIncoming() const;
	void SetIncoming(bool bIncoming);

	void SetAutoPrefix(bool bAutoPrefix);
	bool GetAutoPrefix() const;

protected:
	void Clear();

	bool m_bIncoming;
	UINT m_idEvent;
	String m_sEvent;
	String m_sPrefix;
	Vector<String> m_vecParams;
	bool m_bAutoPrefix;
};

class INetworkEventNotify
{
public:
	virtual void OnEvent(const NetworkEvent& event) = 0;

protected:
	~INetworkEventNotify(){}
};

typedef enum _EVENT_IRC_REPLY
{
	IRC_RPL_WELCOME = 1,
	IRC_RPL_USERHOST = 302,
	IRC_RPL_LISTSTART = 321,
	IRC_RPL_LIST = 322,
	IRC_RPL_LISTEND = 323,
	IRC_RPL_TOPIC = 332,
	IRC_RPL_NAMREPLY = 353,
	IRC_RPL_ENDOFNAMES = 366,
	IRC_RPL_MOTD = 372,

	IRC_ERR_CANNOTSENDTOCHAN = 404, //"<channel name> :Cannot send to channel"
	IRC_ERR_USERNOTINCHANNEL = 441, //"<nick> <channel> :They aren't on that channel"
	IRC_ERR_NOTONCHANNEL = 442, //"<channel> :You're not on that channel"
	IRC_ERR_USERONCHANNEL = 443, //"<user> <channel> :is already on channel"
	IRC_ERR_KEYSET = 467, //"<channel> :Channel key already set"
	IRC_ERR_CHANNELISFULL = 471, //"<channel> :Cannot join channel (+l)"
	IRC_ERR_UNKNOWNMODE = 472, //"<char> :is unknown mode char to me for <channel>"
	IRC_ERR_INVITEONLYCHAN = 473, //"<channel> :Cannot join channel (+i)"
	IRC_ERR_BANNEDFROMCHAN = 474, //"<channel> :Cannot join channel (+b)"
	IRC_ERR_BADCHANNELKEY = 475, //"<channel> :Cannot join channel (+k)"
	IRC_ERR_BADCHANMASK = 476, //"<channel> :Bad Channel Mask"
	IRC_ERR_NOCHANMODES = 477, //"<channel> :Channel doesn't support modes"
	IRC_ERR_BANLISTFULL = 478, //"<channel> <char> :Channel list is full"
	IRC_ERR_NOPRIVILEGES = 481, //":Permission Denied- You're not an IRC operator"
	IRC_ERR_CHANOPRIVSNEEDED = 482, //"<channel> :You're not channel operator"

	IRC_RPL_UNKNOWN = 999,
} EVENT_IRC_REPLY;

typedef enum _EVENT_IRC_COMMAND
{
	IRC_CMD_NICK = EVENT_OFFSET_COMMAND,
	IRC_CMD_USER,
	IRC_CMD_PASS,
	IRC_CMD_JOIN,
	IRC_CMD_PART,
	IRC_CMD_KICK,
	IRC_CMD_INVITE,
	IRC_CMD_PRIVMSG,
	IRC_CMD_NOTICE,
	IRC_CMD_MODE,
	IRC_CMD_PING,
	IRC_CMD_PONG,
	IRC_CMD_TOPIC,
	IRC_CMD_WHOIS,
	IRC_CMD_WHOWAS,
	IRC_CMD_NAMES,
	IRC_CMD_LIST,
	IRC_CMD_USERHOST,
	IRC_CMD_QUIT,
	IRC_CMD_AWAY,
	IRC_CMD_ERROR,
	IRC_CMD_UNKNOWN,
	IRC_CMD_INVALID = -1,
} EVENT_IRC_COMMAND;

typedef enum _EVENT_CTCP_COMMAND
{
	IRC_CTCP_ACTION = EVENT_OFFSET_CTCP,
	IRC_CTCP_PING,
	IRC_CTCP_VERSION,
	IRC_CTCP_DCC,
	IRC_CTCP_UNKNOWN,
	IRC_CTCP_INVALID = -1,
} EVENT_CTCP_COMMAND;
// ^- these need to be synced -v
typedef enum _EVENT_CTCP_REPLY
{
	IRC_CTCP_RPL_ACTION = EVENT_OFFSET_CTCP_REPLY,
	IRC_CTCP_RPL_PING,
	IRC_CTCP_RPL_VERSION,
	IRC_CTCP_RPL_DCC,
	IRC_CTCP_RPL_UNKNOWN,
	IRC_CTCP_RPL_INVALID = -1,
} EVENT_CTCP_REPLY;

typedef enum _EVENT_SYSTEM_MESSAGE
{
	SYS_EVENT_TRYCONNECT = EVENT_OFFSET_SYSTEM,
	SYS_EVENT_WAITCONNECT,
	SYS_EVENT_CONNECTED,
	SYS_EVENT_CONNECTFAILED,
	SYS_EVENT_CLOSE,
	SYS_EVENT_STATUS,
	SYS_EVENT_IDENT,
	SYS_EVENT_ERROR,
	SYS_EVENT_UNKNOWN,
} EVENT_SYSTEM_MESSAGE;

#endif//_NETWORKEVENT_H_INCLUDED_
