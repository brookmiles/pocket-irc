#ifndef _SESSION_H_INCLUDED_
#define _SESSION_H_INCLUDED_

#include "NetworkEvent.h"
#include "Channel.h"

#include "IWriteNetworkEvent.h"

class Session : 
	public INetworkEventNotify
{
public:
	struct ChannelListEntry
	{
		tstring name;
		tstring users;
		tstring topic;
	};

	Session();
	~Session();

	void SetWriter(IWriteNetworkEvent* pWriter);

	void AddEventHandler(INetworkEventNotify* pHandler);
	void RemoveEventHandler(INetworkEventNotify* pHandler);

	void Pass(const tstring& sPass);
	void User(const tstring& sNick, const tstring& sName);
	void Nick(const tstring& sNick);
	void Join(const tstring& sChannel, const tstring& sKey = tstring(0));
	void Part(const tstring& sChannel, const tstring& sMsg = tstring(0));
	void Kick(const tstring& sChannel, const tstring& sUser, const tstring& sReason = tstring(0));
	void Invite(const tstring& sUser, const tstring& sChannel);
	void Topic(const tstring& sChannel, const tstring& sTopic = tstring(0));
	void PrivMsg(const tstring& sTarget, const tstring& sMsg);
	void Action(const tstring& sTarget, const tstring& sMsg);
	void Notice(const tstring& sTarget, const tstring& sMsg);
	void CTCP(const tstring& sTarget, const tstring& sCmd, const tstring& sMsg = tstring(0));
	void CTCPReply(const tstring& sTarget, const tstring& sCmd, const tstring& sMsg = tstring(0));
	void CTCPPing(const tstring& sTarget);
	void Whois(const tstring& sTarget);
	void Mode(const tstring& sTarget, const tstring& sModes);
	void UserHost(const tstring& sUser);
	void Quit(const tstring& sMsg);
	void Away(const tstring& sMsg);
	void Raw(const tstring& sText);
	void Ping();
	void List();

	const tstring& GetNick() const;
	void SetNick(const tstring& sNick);
	bool IsMe(const tstring& sNick);
	bool IsConnected() { return m_bConnected; }
	Channel* GetChannel(const tstring& sChannel) const;
	const std::vector<ChannelListEntry*>& GetChannelList() { return m_vecChannelList; }
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

	Channel* CreateChannelObject(const tstring& sChannel);
	void DestroyChannelObject(const tstring& sChannel);

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
	tstring m_sNick;
	std::vector<Channel*> m_vecChannels;
	std::vector<ChannelListEntry*> m_vecChannelList;

	std::vector<INetworkEventNotify*> m_vecEventHandlers;

	IWriteNetworkEvent* m_pWriter;

	DWORD m_lastActivity;
	bool m_bConnected;
};

#endif//_SESSION_H_INCLUDED_
