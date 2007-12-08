#include "PocketIRC.h"

#include "Channels.h"

FavouriteChannelList::FavouriteChannelList()
{
}

FavouriteChannelList::FavouriteChannelList(FavouriteChannelList& from)
{
	*this = from;
}

FavouriteChannelList::~FavouriteChannelList()
{
	Clear();
}

FavouriteChannelList& FavouriteChannelList::operator=(FavouriteChannelList& from)
{
	Clear();

	UINT nChans = from.Count();
	for(UINT i = 0; i < nChans; ++i)
	{
		FavouriteChannel* chan = from.GetChannel(i);
		_ASSERTE(chan);

		AddChannel(chan->Name, chan->Key, chan->AutoJoin);
	}
	return *this;
}

FavouriteChannel* FavouriteChannelList::GetChannel(UINT i)
{
	_ASSERTE(i < m_listChannels.size());
	if(i < m_listChannels.size())
	{
		UINT j = 0;
		for(chanlist_t::iterator it = m_listChannels.begin(); it != m_listChannels.end(); ++it, ++j)
		{
			if(i == j)
			{
				return *it;
			}
		}
	}
	return NULL;
}

FavouriteChannel* FavouriteChannelList::AddChannel(const tstring& name, const tstring& key, bool autoJoin)
{
	FavouriteChannel* chan = new FavouriteChannel(name, key, autoJoin);
	m_listChannels.push_back(chan);
	return chan;
}

FavouriteChannel* FavouriteChannelList::FindChannel(const tstring& name)
{
	for(chanlist_t::iterator it = m_listChannels.begin(); it != m_listChannels.end(); ++it)
	{
		if((*it)->Name == name)
			return *it;
	}
	return NULL;
}

bool FavouriteChannelList::RemoveChannel(const tstring& name)
{
	FavouriteChannel* chan = FindChannel(name);
	if(chan)
	{
		m_listChannels.remove(chan);
		delete chan;
		return true;
	}
	return false;
}

bool FavouriteChannelList::RemoveChannel(FavouriteChannel* chan)
{
	_ASSERTE(chan);

	m_listChannels.remove(chan);
	delete chan;
	return true;
}

UINT FavouriteChannelList::Count()
{	
	return m_listChannels.size();
}

void FavouriteChannelList::Clear()
{	
	for(chanlist_t::iterator it = m_listChannels.begin(); it != m_listChannels.end(); ++it)
	{
		delete *it;
	}
	m_listChannels.clear();
}
