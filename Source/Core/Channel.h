#ifndef _CHANNEL_H_INCLUDED_
#define _CHANNEL_H_INCLUDED_

#include "NetworkEvent.h"
#include "NickList.h"

class Session;

class Channel
{
public:
	Channel();
	~Channel();

	void SetName(const tstring& sChannel);
	void SetSession(Session* pSession) { m_pSession = pSession; }

	void ParseNameList(const tstring& sNameList);
	void AddName(const tstring& sName);
	void RemoveName(const tstring& sName);
	void ChangeName(const tstring& sOldName, const tstring& sNewName);

	void OnMode(const NetworkEvent& event);

//IChannel
	const tstring& GetName() const;
	bool IsOn(const tstring& sUser);
	bool IsVoice(const tstring& sUser);
	bool IsOp(const tstring& sUser);
	tstring GetHighestUserMode(const tstring& sUser);

protected:

private:
	Session* m_pSession;
	tstring m_sChannel;

	NickList m_nickList;
	//IChannelUserNotify* m_pNotify;

};

struct ChannelMode
{
	bool add;
	TCHAR mode;
	tstring param;
};

void ParseChannelModes(const NetworkEvent& eventIn, std::vector<ChannelMode>& listOut);

#endif//_CHANNEL_H_INCLUDED_
