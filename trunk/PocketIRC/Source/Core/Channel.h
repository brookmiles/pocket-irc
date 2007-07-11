#ifndef _CHANNEL_H_INCLUDED_
#define _CHANNEL_H_INCLUDED_

#include "StringT.h"

#include "NetworkEvent.h"
#include "NickList.h"

class Session;

class Channel
{
public:
	Channel();
	~Channel();

	void SetName(const String& sChannel);
	void SetSession(Session* pSession);

	void ParseNameList(const String& sNameList);
	void AddName(const String& sName, bool bOp, bool bVoice);
	void RemoveName(const String& sName);
	void ChangeName(const String& sOldName, const String& sNewName);

	void OnMode(const NetworkEvent& event);

//IChannel
	const String& GetName() const;
	bool IsOn(const String& sUser);
	bool IsVoice(const String& sUser);
	bool IsOp(const String& sUser);

protected:

private:
	Session* m_pSession;
	String m_sChannel;

	NickList m_nickList;
	//IChannelUserNotify* m_pNotify;

};

struct ChannelMode
{
	bool add;
	TCHAR mode;
	String param;
};

void ParseChannelModes(const NetworkEvent& eventIn, Vector<ChannelMode>& listOut);

#endif//_CHANNEL_H_INCLUDED_
