#ifndef _SESSION_H_INCLUDED_
#define _SESSION_H_INCLUDED_

#include "StringT.h"

#include "NetworkEvent.h"
#include "Channel.h"

#include "IWriteNetworkEvent.h"

class Session : 
	public INetworkEventNotify
{
public:
	struct ChannelListEntry
	{
		String name;
		String users;
		String topic;
	};

	Session();
	~Session();

	void SetWriter(IWriteNetworkEvent* pWriter);

	void AddEventHandler(INetworkEventNotify* pHandler);
	void RemoveEventHandler(INetworkEventNotify* pHandler);

	void Pass(const String& sPass);
	void User(const String& sNick, const String& sName);
	void Nick(const String& sNick);
	void Join(const String& sChannel, const String& sKey = String(0));
	void Part(const String& sChannel, const String& sMsg = String(0));
	void Kick(const String& sChannel, const String& sUser, const String& sReason = String(0));
	void Invite(const String& sUser, const String& sChannel);
	void Topic(const String& sChannel, const String& sTopic = String(0));
	void PrivMsg(const String& sTarget, const String& sMsg);
	void Action(const String& sTarget, const String& sMsg);
	void Notice(const String& sTarget, const String& sMsg);
	void CTCP(const String& sTarget, const String& sCmd, const String& sMsg = String(0));
	void CTCPReply(const String& sTarget, const String& sCmd, const String& sMsg = String(0));
	void CTCPPing(const String& sTarget);
	void Whois(const String& sTarget);
	void Mode(const String& sTarget, const String& sModes);
	void UserHost(const String& sUser);
	void Quit(const String& sMsg);
	void Away(const String& sMsg);
	void Raw(const String& sText);
	void Ping();
	void List();

	const String& GetNick() const;
	void SetNick(const String& sNick);
	bool IsMe(const String& sNick);
	bool IsConnected() { return m_bConnected; }
	Channel* GetChannel(const String& sChannel) const;
	const Vector<ChannelListEntry*>& GetChannelList() { return m_vecChannelList; }
	void ClearChannelList();

	DWORD GetIdleTime() { return GetTickCount() - m_lastActivity; }

//INetworkEventNotify
	// OnEvent - Accept an incoming event
	void OnEvent(const NetworkEvent& networkEvent);

protected:
	void ResetIdleCounter() { m_lastActivity = GetTickCount(); }
	
	// DispatchEvent - Send an event to the display handlers
	void DispatchEvent(const NetworkEvent& networkEvent);

	// DoEvent - Send an outgoing event
	void DoEvent(const NetworkEvent& networkEvent);

	Channel* CreateChannelObject(const String& sChannel);
	void DestroyChannelObject(const String& sChannel);

	void OnConnect(const NetworkEvent& event);
	void OnDisconnect(const NetworkEvent& event);

	void OnJoin(const NetworkEvent& event);
	void OnPart(const NetworkEvent& event);
	void OnKick(const NetworkEvent& event);
	void OnNick(const NetworkEvent& event);
	void OnQuit(const NetworkEvent& event);
	void OnPing(const NetworkEvent& event);
	void OnMode(const NetworkEvent& event);

	void OnCTCPPing(const NetworkEvent& event);
	void OnCTCPVersion(const NetworkEvent& event);
	void OnCTCPRplPing(const NetworkEvent& event);

	void OnRplNamReply(const NetworkEvent& event);
	void OnRplWelcome(const NetworkEvent& event);
	void OnRplListStart(const NetworkEvent& event);
	void OnRplList(const NetworkEvent& event);

private:
	String m_sNick;
	Vector<Channel*> m_vecChannels;
	Vector<ChannelListEntry*> m_vecChannelList;

	Vector<INetworkEventNotify*> m_vecEventHandlers;

	IWriteNetworkEvent* m_pWriter;

	DWORD m_lastActivity;
	bool m_bConnected;
};

#endif//_SESSION_H_INCLUDED_
