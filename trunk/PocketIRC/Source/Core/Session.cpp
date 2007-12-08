#include "PocketIRC.h"
#include "Session.h"

#include "IrcString.h"
#include "IrcStringFormat.h"
#include "StringUtil.h"
#include "VectorUtil.h"

/////////////////////////////////////////////////////////////////////////////
// Constructors/Destructors
/////////////////////////////////////////////////////////////////////////////

Session::Session() :
	m_pWriter(NULL),
	m_sNick(_T("")),
	m_bConnected(false)
{
	ResetIdleCounter();
}

Session::~Session()
{
	ClearChannelList();
}

/////////////////////////////////////////////////////////////////////////////
// Interface
/////////////////////////////////////////////////////////////////////////////

void Session::SetWriter(IWriteNetworkEvent* pWriter)
{
	m_pWriter = pWriter;
}

void Session::SetNick(const tstring& sNick)
{
	m_sNick = sNick;
}

void Session::AddEventHandler(INetworkEventNotify* pHandler)
{
	_ASSERTE(pHandler != NULL);

	std::vector<INetworkEventNotify*>::iterator i = std::find(m_vecEventHandlers.begin(), m_vecEventHandlers.end(), pHandler);
	if(i == m_vecEventHandlers.end())
	{
		m_vecEventHandlers.push_back(pHandler);
	}
}

void Session::RemoveEventHandler(INetworkEventNotify* pHandler)
{
	_ASSERTE(pHandler != NULL);

	Erase(m_vecEventHandlers, pHandler);
}

const tstring& Session::GetNick() const
{
	return m_sNick;
}

void Session::Pass(const tstring& sPass)
{
	NetworkEvent event(IRC_CMD_PASS, 1, &sPass);
	event.SetAutoPrefix(false);
	DoEvent(event);
}

void Session::User(const tstring& sNick, const tstring& sName)
{
	SetNick(sNick);
	NetworkEvent event(IRC_CMD_USER, 4, &sNick, &tstring(_T("\"pocketirc.com\"")), &tstring(_T("\"pocketirc.com\"")), &sName);
	DoEvent(event);
}

void Session::Nick(const tstring& sNick)
{
	NetworkEvent event(IRC_CMD_NICK, 1, &sNick);
	event.SetAutoPrefix(false);
	DoEvent(event);
}

void Session::Join(const tstring& sChannel, const tstring& sKey)
{
	NetworkEvent event(IRC_CMD_JOIN, 1, &sChannel);

	if(sKey.size())
	{
		event.AddParam(sKey);
	}

	DoEvent(event);
}

void Session::Part(const tstring& sChannel, const tstring& sMsg)
{
	NetworkEvent event(IRC_CMD_PART, 1, &sChannel);
	if(sMsg.size())
	{
		event.AddParam(sMsg);
	}
	DoEvent(event);
}

void Session::Kick(const tstring& sChannel, const tstring& sUser, const tstring& sReason)
{
	NetworkEvent event(IRC_CMD_KICK, 2, &sChannel, &sUser);

	if(sReason.size())
	{
		event.AddParam(sReason);
	}

	DoEvent(event);
}

void Session::Invite(const tstring& sUser, const tstring& sChannel)
{
	NetworkEvent event(IRC_CMD_INVITE, 2, &sUser, &sChannel);
	DoEvent(event);
}

void Session::Topic(const tstring& sChannel, const tstring& sTopic)
{
	NetworkEvent event(IRC_CMD_TOPIC, 1, &sChannel);

	if(sTopic.size())
	{
		event.AddParam(sTopic);
	}

	DoEvent(event);
}

void Session::PrivMsg(const tstring& sTarget, const tstring& sMsg)
{
	NetworkEvent event(IRC_CMD_PRIVMSG, 2, &sTarget, &sMsg);
	DoEvent(event);
}

void Session::Action(const tstring& sTarget, const tstring& sMsg)
{
	CTCP(sTarget, _T("ACTION"), sMsg);
}

void Session::Notice(const tstring& sTarget, const tstring& sMsg)
{
	NetworkEvent event(IRC_CMD_NOTICE, 2, &sTarget, &sMsg);
	DoEvent(event);
}

void Session::CTCP(const tstring& sTarget, const tstring& sCmd, const tstring& sMsg)
{
	int idEvent = NetworkEvent::CTCPEventStringToID(sCmd);
	NetworkEvent event(idEvent, 1, &sTarget);
	event.SetEvent(sCmd);

	if(sMsg.size())
	{
		event.AddParam(sMsg);
	}

	DoEvent(event);
}

void Session::CTCPPing(const tstring& sTarget)
{
	NetworkEvent event(IRC_CTCP_PING, 1, &sTarget);
	event.SetEvent(NetworkEvent::EventIDToString(IRC_CTCP_PING));

	const int MAGIC_NUMBER = 32;
	TCHAR buf[MAGIC_NUMBER];
	DWORD t = GetTickCount() / 1000;
	_sntprintf(buf, MAGIC_NUMBER, _T("%u"), t);
	buf[MAGIC_NUMBER - 1] = 0;

	event.AddParam(buf);

	DoEvent(event);
}

void Session::CTCPReply(const tstring& sTarget, const tstring& sCmd, const tstring& sMsg)
{
	int idEvent = NetworkEvent::CTCPEventStringToID(sCmd, true);
	if(idEvent == IRC_CTCP_RPL_INVALID)
	{
		_TRACE("Session(0x%08X)::CTCPReply(\"%s\", \"%s\") INVALID COMMAND", this, sCmd.c_str(), sMsg.c_str());
	}
	else
	{
		NetworkEvent event(idEvent, 1, &sTarget);

		if(sMsg.size())
		{
			event.AddParam(sMsg);
		}

		DoEvent(event);
	}
}

void Session::Whois(const tstring& sTarget)
{
	NetworkEvent event(IRC_CMD_WHOIS, 1, &sTarget);
	DoEvent(event);
}

void Session::Mode(const tstring& sTarget, const tstring& sModes)
{
	NetworkEvent event(IRC_CMD_MODE, 1, &sTarget);
	
	std::vector<tstring> modes;
	Split(sModes, modes, _T(' '), false);

	for(std::vector<tstring>::iterator i = modes.begin(); i != modes.end(); ++i)
	{
		event.AddParam(*i);
	}

	DoEvent(event);
}

void Session::UserHost(const tstring& sUser)
{
	NetworkEvent event(IRC_CMD_USERHOST, 1, &sUser);
	DoEvent(event);
}

void Session::Quit(const tstring& sMsg)
{
	NetworkEvent event(IRC_CMD_QUIT, 1, &sMsg);
	DoEvent(event);
}

void Session::Away(const tstring& sMsg)
{
	NetworkEvent event(IRC_CMD_AWAY, 1, &sMsg);
	DoEvent(event);
}

void Session::Ping()
{
	NetworkEvent event(IRC_CMD_PING, 1, &GetNick());
	DoEvent(event);
}

void Session::List()
{
	NetworkEvent event(IRC_CMD_LIST, 0);
	DoEvent(event);
}

void Session::Raw(const tstring& sText)
{
	ResetIdleCounter();

	tstring sLine = sText;
	m_pWriter->Write(sLine);
}


/////////////////////////////////////////////////////////////////////////////
// Internal Methods
/////////////////////////////////////////////////////////////////////////////

bool Session::IsMe(const tstring& sNick)
{
	return (Compare(m_sNick, sNick, false));
}

// DispatchEvent - Send an event to the display handlers, or whatever.
void Session::DispatchEvent(const NetworkEvent& networkEvent)
{
	//_TRACE("Session(0x%08X)::DispatchEvent(\"%s\")", this, networkEvent.GetEvent().c_str());

	for(UINT i = 0; i < m_vecEventHandlers.size(); ++i)
	{
		INetworkEventNotify* pHandler = m_vecEventHandlers[i];
		_ASSERTE(pHandler != NULL);

		pHandler->OnEvent(networkEvent);
	}
}

// DoEvent - Send an outgoing event
void Session::DoEvent(const NetworkEvent& networkEvent)
{
	_TRACE("Session(0x%08X)::DoEvent(\"%s\")", this, networkEvent.GetEvent().c_str());

	ResetIdleCounter();

	m_pWriter->WriteEvent(networkEvent);
	DispatchEvent(networkEvent);
}

/////////////////////////////////////////////////////////////////////////////
// Channel and Query Object Management
/////////////////////////////////////////////////////////////////////////////

Channel* Session::GetChannel(const tstring& sChannel) const
{
	for(UINT i = 0; i < m_vecChannels.size(); ++i)
	{
		if(Compare(sChannel, m_vecChannels[i]->GetName(), false))
			return m_vecChannels[i]; 
	}
	return NULL;
}

Channel* Session::CreateChannelObject(const tstring& sChannel)
{
	Channel* pChan = new Channel();

	pChan->SetName(sChannel);

	m_vecChannels.push_back(pChan);

	return pChan;
}

void Session::DestroyChannelObject(const tstring& sChannel)
{
	Channel* pChannel = GetChannel(sChannel);

	Erase(m_vecChannels, pChannel);

	delete pChannel;
}

/////////////////////////////////////////////////////////////////////////////
// INetworkEventNotify
/////////////////////////////////////////////////////////////////////////////

#define HANDLE_IRC_EVENT(e, h) case e: h(event); break;

void Session::OnEvent(const NetworkEvent& event)
{
	ResetIdleCounter();

	// Phase 1.  Events that should be handled before being displayed
	switch(event.GetEventID())
	{
		HANDLE_IRC_EVENT(IRC_CMD_NICK, OnNick)
		HANDLE_IRC_EVENT(IRC_CMD_JOIN, OnJoin)
		HANDLE_IRC_EVENT(IRC_CMD_PART, OnPart)
		HANDLE_IRC_EVENT(IRC_CMD_KICK, OnKick)
		HANDLE_IRC_EVENT(IRC_CMD_MODE, OnMode)
		HANDLE_IRC_EVENT(IRC_CMD_QUIT, OnQuit)

		HANDLE_IRC_EVENT(IRC_RPL_WELCOME, OnRplWelcome)
		HANDLE_IRC_EVENT(IRC_RPL_NAMREPLY, OnRplNamReply)
		HANDLE_IRC_EVENT(IRC_RPL_LISTSTART, OnRplListStart)
		HANDLE_IRC_EVENT(IRC_RPL_LIST, OnRplList)

		HANDLE_IRC_EVENT(IRC_CTCP_RPL_PING, OnCTCPRplPing)
	}

	// Phase 2. Dispatch Events to external handlers
	DispatchEvent(event);

	// Phase 3. Events that should be handled after being handled.... uh....
	switch(event.GetEventID())
	{
		HANDLE_IRC_EVENT(IRC_CMD_PING, OnPing)

		HANDLE_IRC_EVENT(IRC_CTCP_PING, OnCTCPPing)
		HANDLE_IRC_EVENT(IRC_CTCP_VERSION, OnCTCPVersion)

		HANDLE_IRC_EVENT(SYS_EVENT_CONNECTED, OnConnect)
		HANDLE_IRC_EVENT(SYS_EVENT_CLOSE, OnDisconnect)
	}
}

/////////////////////////////////////////////////////////////////////////////
// Session state handlers
/////////////////////////////////////////////////////////////////////////////

void Session::OnConnect(const NetworkEvent& event)
{
	m_bConnected = true;
}

void Session::OnDisconnect(const NetworkEvent& event)
{
	m_bConnected = false;

	for(UINT i = 0; i < m_vecChannels.size(); ++i)
	{
		delete m_vecChannels[i];
	}

	m_vecChannels.clear();
}

void Session::OnJoin(const NetworkEvent& event)
{
	tstring sUser = GetPrefixNick(event.GetPrefix());
	const tstring& sChannel = event.GetParam(0);

	if(IsMe(sUser))
	{
		Channel* pChan = CreateChannelObject(sChannel);
	}
	else
	{
		Channel* pChan = GetChannel(sChannel);
		_ASSERTE(pChan != NULL);

		if(pChan != NULL)
		{
			pChan->AddName(sUser, false, false);
		}
	}
}

void Session::OnPart(const NetworkEvent& event)
{
	tstring sUser = GetPrefixNick(event.GetPrefix());
	const tstring& sChannel = event.GetParam(0);

	if(IsMe(sUser))
	{
		DestroyChannelObject(sChannel);
	}
	else
	{
		Channel* pChan = GetChannel(sChannel);
		_ASSERTE(pChan != NULL);

		if(pChan != NULL)
		{
			pChan->RemoveName(sUser);
		}
	}
}

void Session::OnKick(const NetworkEvent& event)
{
	const tstring& sChannel = event.GetParam(0);
	const tstring& sUser = event.GetParam(1);

	if(IsMe(sUser))
	{
		DestroyChannelObject(sChannel);
	}
	else
	{
		Channel* pChan = GetChannel(sChannel);
		_ASSERTE(pChan != NULL);

		if(pChan != NULL)
		{
			pChan->RemoveName(sUser);
		}
	}
}

void Session::OnNick(const NetworkEvent& event)
{
	tstring sUser = GetPrefixNick(event.GetPrefix());
	const tstring& sNick = event.GetParam(0);

	if(IsMe(sUser))
	{
		SetNick(sNick);
	}

	// Update channel nick lists
	if(m_vecChannels.size() > 0)
	{
		//Change user's nick in channel nick lists
		for(UINT iChan = 0; iChan < m_vecChannels.size(); ++iChan)
		{
			Channel* pChan = m_vecChannels[iChan];
			_ASSERTE(pChan != NULL);

			if(pChan != NULL)
			{
				if(pChan->IsOn(sUser))
				{
					pChan->ChangeName(sUser, sNick);
				}
			}
		}
	}
}

void Session::OnQuit(const NetworkEvent& event)
{
	tstring sUser = GetPrefixNick(event.GetPrefix());

	if(IsMe(sUser))
	{
		// Don't think we ever get QUIT for ourself
	}
	else
	{
		for(UINT iChan = 0; iChan < m_vecChannels.size(); ++iChan)
		{
			Channel* pChan = m_vecChannels[iChan];
			_ASSERTE(pChan != NULL);

			if(pChan != NULL)
			{
				if(pChan->IsOn(sUser))
				{
					pChan->RemoveName(sUser);
				}
			}
		}
	}
}

void Session::OnPing(const NetworkEvent& event)
{
	NetworkEvent out(IRC_CMD_PONG, 1, &event.GetParam(0));

	DoEvent(out);
}

void Session::OnCTCPPing(const NetworkEvent& event)
{
	tstring from = GetPrefixNick(event.GetPrefix());

	CTCPReply(from, _T("PING"), event.GetParam(1));
}

void Session::OnCTCPRplPing(const NetworkEvent& event)
{
	tstring from = GetPrefixNick(event.GetPrefix());
	tstring sTime = event.GetParam(1);

	if(sTime.size())
	{
		DWORD now = GetTickCount() / 1000;
		DWORD then = _tcstoul(sTime.c_str(), NULL, 10);
		DWORD seconds = now - then;

		TCHAR buf[30];
		wsprintf(buf, _T("%u"), seconds);

		sTime = buf;

		const_cast<NetworkEvent&>(event).AddParam(buf); // zomg bad
	}
}

void Session::OnCTCPVersion(const NetworkEvent& event)
{
	tstring from = GetPrefixNick(event.GetPrefix());

	CTCPReply(from, _T("VERSION"), POCKETIRC_VERSION_REPLY);
}

void Session::OnRplNamReply(const NetworkEvent& event)
{
	const tstring& sChannel = event.GetParam(2);
	_ASSERTE(sChannel.size());

	Channel* pChannel = GetChannel(sChannel);

	if(pChannel)
	{
		pChannel->ParseNameList(event.GetParam(3));
	}
}

void Session::OnMode(const NetworkEvent& event)
{
	const tstring& sTarget = event.GetParam(0);
	_ASSERTE(sTarget.size());

	if(IsChannel(sTarget))
	{
		Channel* pChannel = GetChannel(sTarget);

		if(pChannel)
		{
			pChannel->OnMode(event);
		}
	}
}

void Session::OnRplWelcome(const NetworkEvent& event)
{
	const tstring& sMe = event.GetParam(0);
	_ASSERTE(sMe.size());

	SetNick(sMe);
}


void Session::OnRplListStart(const NetworkEvent& event)
{
	ClearChannelList();
}

void Session::OnRplList(const NetworkEvent& event)
{
	tstring sChannel = event.GetParam(1);
	tstring sUsers = event.GetParam(2);
	tstring sTopic = event.GetParam(3);

	if(sTopic[0] == '[' && sTopic[sTopic.size() - 1] == ']')
		sTopic = _T("");
	sTopic = StringFormat::StripFormatting(sTopic);

	if(sChannel != _T("*"))
	{
		ChannelListEntry* chan = new ChannelListEntry();
		chan->name = sChannel;
		chan->users = sUsers;
		chan->topic = sTopic;

		m_vecChannelList.push_back(chan);
	}
}

void Session::ClearChannelList()
{
	for(UINT i = 0; i < m_vecChannelList.size(); ++i)
	{
		delete m_vecChannelList[i];
	}
	m_vecChannelList.clear();
}

