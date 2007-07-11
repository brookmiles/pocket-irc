#pragma once

#include "Common\StringT.h"
#include "Common\Vector.h"

struct FavouriteChannel
{
	FavouriteChannel(const String& name, const String& key = _T(""), bool autoJoin = false)
	{
		Name = name;
		Key = key;
		AutoJoin = autoJoin;
	}
	String Name;
	String Key;
	bool AutoJoin;
};

class FavouriteChannelList
{
public:
	FavouriteChannelList();
	FavouriteChannelList(FavouriteChannelList& from);
	~FavouriteChannelList();

	FavouriteChannelList& operator=(FavouriteChannelList& from);

	FavouriteChannel* GetChannel(UINT i);
	FavouriteChannel* AddChannel(const String& name, const String& key = _T(""), bool autoJoin = false);
	FavouriteChannel* FindChannel(const String& name);
	bool RemoveChannel(const String& name);
	bool RemoveChannel(FavouriteChannel* chan);
	UINT Count();
	void Clear();

private:
	typedef std::list<FavouriteChannel*> chanlist_t;
	chanlist_t m_listChannels;

};
