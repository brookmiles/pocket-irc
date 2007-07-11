#include "PocketIRC.h"
#include "Session.h"

#include "IrcString.h"
#include "IrcStringFormat.h"

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

void Session::SetNick(const String& sNick)
{
	m_sNick = sNick;
}

void Session::AddEventHandler(INetworkEventNotify* pHandler)
{
	_ASSERTE(pHandler != NULL);

	UINT index = m_vecEventHandlers.Find(pHandler);
	if(index == -1)
	{
		m_vecEventHandlers.Append(pHandler);
	}
}

void Session::RemoveEventHandler(INetworkEventNotify* pHandler)
{
	_ASSERTE(pHandler != NULL);

	UINT index = m_vecEventHandlers.Find(pHandler);
	if(index != -1)
	{
		m_vecEventHandlers.Erase(index);
	}
}

const String& Session::GetNick() const
{
	return m_sNick;
}

void Session::Pass(const String& sPass)
{
	NetworkEvent event(IRC_CMD_PASS, 1, &sPass);
	event.SetAutoPrefix(false);
	DoEvent(event);
}

void Session::User(const String& sNick, const String& sName)
{
	SetNick(sNick);
	NetworkEvent event(IRC_CMD_USER, 4, &sNick, &String(_T("\"pocketirc.com\"")), &String(_T("\"pocketirc.com\"")), &sName);
	DoEvent(event);
}

void Session::Nick(const String& sNick)
{
	NetworkEvent event(IRC_CMD_NICK, 1, &sNick);
	event.SetAutoPrefix(false);
	DoEvent(event);
}

void Session::Join(const String& sChannel, const String& sKey)
{
	NetworkEvent event(IRC_CMD_JOIN, 1, &sChannel);

	if(sKey.Size())
	{
		event.AddParam(sKey);
	}

	DoEvent(event);
}

void Session::Part(const String& sChannel, const String& sMsg)
{
	NetworkEvent event(IRC_CMD_PART, 1, &sChannel);
	if(sMsg.Size())
	{
		event.AddParam(sMsg);
	}
	DoEvent(event);
}

void Session::Kick(const String& sChannel, const String& sUser, const String& sReason)
{
	NetworkEvent event(IRC_CMD_KICK, 2, &sChannel, &sUser);

	if(sReason.Size())
	{
		event.AddParam(sReason);
	}

	DoEvent(event);
}

void Session::Invite(const String& sUser, const String& sChannel)
{
	NetworkEvent event(IRC_CMD_INVITE, 2, &sUser, &sChannel);
	DoEvent(event);
}

void Session::Topic(const String& sChannel, const String& sTopic)
{
	NetworkEvent event(IRC_CMD_TOPIC, 1, &sChannel);

	if(sTopic.Size())
	{
		event.AddParam(sTopic);
	}

	DoEvent(event);
}

void Session::PrivMsg(const String& sTarget, const String& sMsg)
{
	NetworkEvent event(IRC_CMD_PRIVMSG, 2, &sTarget, &sMsg);
	DoEvent(event);
}

void Session::Action(const String& sTarget, const String& sMsg)
{
	CTCP(sTarget, _T("ACTION"), sMsg);
}

void Session::Notice(const String& sTarget, const String& sMsg)
{
	NetworkEvent event(IRC_CMD_NOTICE, 2, &sTarget, &sMsg);
	DoEvent(event);
}

void Session::CTCP(const String& sTarget, const String& sCmd, const String& sMsg)
{
	int idEvent = NetworkEvent::CTCPEventStringToID(sCmd);
	NetworkEvent event(idEvent, 1, &sTarget);
	event.SetEvent(sCmd);

	if(sMsg.Size())
	{
		event.AddParam(sMsg);
	}

	DoEvent(event);
}

void Session::CTCPPing(const String& sTarget)
{
	NetworkEvent event(IRC_CTCP_PING, 1, &sTarget);
	event.SetEvent(NetworkEvent::EventIDToString(IRC_CTCP_PING));

	const int MAGIC_NUMBER = 32;
	String sTime;
	sTime.Reserve(MAGIC_NUMBER);
	DWORD t = GetTickCount() / 1000;
	_sntprintf(sTime.Str(), MAGIC_NUMBER, _T("%u"), t);
	sTime.Str()[MAGIC_NUMBER - 1] = 0;

	event.AddParam(sTime);

	DoEvent(event);
}

void Session::CTCPReply(const String& sTarget, const String& sCmd, const String& sMsg)
{
	int idEvent = NetworkEvent::CTCPEventStringToID(sCmd, true);
	if(idEvent == IRC_CTCP_RPL_INVALID)
	{
		_TRACE("Session(0x%08X)::CTCPReply(\"%s\", \"%s\") INVALID COMMAND", this, sCmd.Str(), sMsg.Str());
	}
	else
	{
		NetworkEvent event(idEvent, 1, &sTarget);

		if(sMsg.Size())
		{
			event.AddParam(sMsg);
		}

		DoEvent(event);
	}
}

void Session::Whois(const String& sTarget)
{
	NetworkEvent event(IRC_CMD_WHOIS, 1, &sTarget);
	DoEvent(event);
}

void Session::Mode(const String& sTarget, const String& sModes)
{
	NetworkEvent event(IRC_CMD_MODE, 1, &sTarget);
	
	String sModeParam;
	UINT iWord = 0;
	while((sModeParam = sModes.GetWord(iWord)).Size())
	{
		event.AddParam(sModeParam);
		iWord++;
	}
	DoEvent(event);
}

void Session::UserHost(const String& sUser)
{
	NetworkEvent event(IRC_CMD_USERHOST, 1, &sUser);
	DoEvent(event);
}

void Session::Quit(const String& sMsg)
{
	NetworkEvent event(IRC_CMD_QUIT, 1, &sMsg);
	DoEvent(event);
}

void Session::Away(const String& sMsg)
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

void Session::Raw(const String& sText)
{
	ResetIdleCounter();

	String sLine = sText;
	sLine.Append(_T("\r\n"));
	m_pWriter->Raw(sLine);
}


/////////////////////////////////////////////////////////////////////////////
// Internal Methods
/////////////////////////////////////////////////////////////////////////////

bool Session::IsMe(const String& sNick)
{
	return (m_sNick.Compare(sNick, false));
}

// DispatchEvent - Send an event to the display handlers, or whatever.
void Session::DispatchEvent(const NetworkEvent& networkEvent)
{
	//_TRACE("Session(0x%08X)::DispatchEvent(\"%s\")", this, networkEvent.GetEvent().Str());

	for(UINT i = 0; i < m_vecEventHandlers.Size(); ++i)
	{
		INetworkEventNotify* pHandler = m_vecEventHandlers[i];
		_ASSERTE(pHandler != NULL);

		pHandler->OnEvent(networkEvent);
	}
}

// DoEvent - Send an outgoing event
void Session::DoEvent(const NetworkEvent& networkEvent)
{
	_TRACE("Session(0x%08X)::DoEvent(\"%s\")", this, networkEvent.GetEvent().Str());

	ResetIdleCounter();

	m_pWriter->WriteEvent(networkEvent);
	DispatchEvent(networkEvent);
}

/////////////////////////////////////////////////////////////////////////////
// Channel and Query Object Management
/////////////////////////////////////////////////////////////////////////////

Channel* Session::GetChannel(const String& sChannel) const
{
	for(UINT i = 0; i < m_vecChannels.Size(); ++i)
	{
		if(sChannel.Compare(m_vecChannels[i]->GetName(), false))
			return m_vecChannels[i]; 
	}
	return NULL;
}

Channel* Session::CreateChannelObject(const String& sChannel)
{
	Channel* pChan = new Channel();

	pChan->SetName(sChannel);

	m_vecChannels.Append(pChan);

	return pChan;
}

void Session::DestroyChannelObject(const String& sChannel)
{
	Channel* pChannel = GetChannel(sChannel);

	UINT index = m_vecChannels.Find(pChannel);
	if(index != Vector<Channel*>::NPOS)
	{
		m_vecChannels.Erase(index);
	}

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

	for(UINT i = 0; i < m_vecChannels.Size(); ++i)
	{
		delete m_vecChannels[i];
	}

	m_vecChannels.Free();
}

void Session::OnJoin(const NetworkEvent& event)
{
	String sUser = GetPrefixNick(event.GetPrefix());
	const String& sChannel = event.GetParam(0);

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
	String sUser = GetPrefixNick(event.GetPrefix());
	const String& sChannel = event.GetParam(0);

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
	const String& sChannel = event.GetParam(0);
	const String& sUser = event.GetParam(1);

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
	String sUser = GetPrefixNick(event.GetPrefix());
	const String& sNick = event.GetParam(0);

	if(IsMe(sUser))
	{
		SetNick(sNick);
	}

	// Update channel nick lists
	if(m_vecChannels.Size() > 0)
	{
		//Change user's nick in channel nick lists
		for(UINT iChan = 0; iChan < m_vecChannels.Size(); ++iChan)
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
	String sUser = GetPrefixNick(event.GetPrefix());

	if(IsMe(sUser))
	{
		// Don't think we ever get QUIT for ourself
	}
	else
	{
		for(UINT iChan = 0; iChan < m_vecChannels.Size(); ++iChan)
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
	String from = GetPrefixNick(event.GetPrefix());

	CTCPReply(from, _T("PING"), event.GetParam(1));
}

void Session::OnCTCPRplPing(const NetworkEvent& event)
{
	String from = GetPrefixNick(event.GetPrefix());
	String sTime = event.GetParam(1);

	if(sTime.Size())
	{
		DWORD now = GetTickCount() / 1000;
		DWORD then = _tcstoul(sTime.Str(), NULL, 10);
		DWORD seconds = now - then;

		TCHAR buf[30];
		wsprintf(buf, _T("%u"), seconds);

		sTime = buf;

		const_cast<NetworkEvent&>(event).AddParam(buf); // zomg bad
	}
}

void Session::OnCTCPVersion(const NetworkEvent& event)
{
	String from = GetPrefixNick(event.GetPrefix());

	CTCPReply(from, _T("VERSION"), POCKETIRC_VERSION_REPLY);
}

void Session::OnRplNamReply(const NetworkEvent& event)
{
	const String& sChannel = event.GetParam(2);
	_ASSERTE(sChannel.Size());

	Channel* pChannel = GetChannel(sChannel);

	if(pChannel)
	{
		pChannel->ParseNameList(event.GetParam(3));
	}
}

void Session::OnMode(const NetworkEvent& event)
{
	const String& sTarget = event.GetParam(0);
	_ASSERTE(sTarget.Size());

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
	const String& sMe = event.GetParam(0);
	_ASSERTE(sMe.Size());

	SetNick(sMe);
}


void Session::OnRplListStart(const NetworkEvent& event)
{
	ClearChannelList();
}

void Session::OnRplList(const NetworkEvent& event)
{
	String sChannel = event.GetParam(1);
	String sUsers = event.GetParam(2);
	String sTopic = event.GetParam(3);

	if(sTopic[0] == '[' && sTopic[sTopic.Size() - 1] == ']')
		sTopic = _T("");
	sTopic = StringFormat::StripFormatting(sTopic);

	if(sChannel != _T("*"))
	{
		ChannelListEntry* chan = new ChannelListEntry();
		chan->name = sChannel;
		chan->users = sUsers;
		chan->topic = sTopic;

		m_vecChannelList.Append(chan);
	}
}

void Session::ClearChannelList()
{
	for(UINT i = 0; i < m_vecChannelList.Size(); ++i)
	{
		delete m_vecChannelList[i];
	}
	m_vecChannelList.Clear();
}

