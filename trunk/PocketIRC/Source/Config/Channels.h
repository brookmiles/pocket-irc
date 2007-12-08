#pragma once

struct FavouriteChannel
{
	FavouriteChannel(const tstring& name, const tstring& key = _T(""), bool autoJoin = false)
	{
		Name = name;
		Key = key;
		AutoJoin = autoJoin;
	}
	tstring Name;
	tstring Key;
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
	FavouriteChannel* AddChannel(const tstring& name, const tstring& key = _T(""), bool autoJoin = false);
	FavouriteChannel* FindChannel(const tstring& name);
	bool RemoveChannel(const tstring& name);
	bool RemoveChannel(FavouriteChannel* chan);
	UINT Count();
	void Clear();

private:
	typedef std::list<FavouriteChannel*> chanlist_t;
	chanlist_t m_listChannels;

};
