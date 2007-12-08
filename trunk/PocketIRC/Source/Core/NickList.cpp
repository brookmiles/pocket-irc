#include "PocketIRC.h"
#include "NickList.h"

/////////////////////////////////////////////////////////////////////////////
// Constructors/Destructors
/////////////////////////////////////////////////////////////////////////////

NickList::NickList()
{

}

NickList::~NickList()
{
	Clear();
}

/////////////////////////////////////////////////////////////////////////////
// Interface
/////////////////////////////////////////////////////////////////////////////

void NickList::AddNick(const tstring& sNick, bool bOp, bool bVoice)
{
	NickListEntry* pEntry = GetEntry(sNick);

	if(pEntry == NULL)
	{
		pEntry = new NickListEntry;
		pEntry->nick = sNick;

		m_vecNicks.push_back(pEntry);
	}

	_ASSERTE(pEntry != NULL);

	pEntry->op = bOp;
	pEntry->voice = bVoice;
}

void NickList::RemoveNick(const tstring& sNick)
{
	UINT i = Find(sNick);
	if(i != -1)
	{
		delete m_vecNicks[i];
		m_vecNicks.erase(m_vecNicks.begin() + i);
	}
}

void NickList::Clear()
{
	for(UINT i = 0; i < m_vecNicks.size(); ++i)
	{
		delete m_vecNicks[i];
	}
	m_vecNicks.clear();
}


UINT NickList::Count()
{
	return m_vecNicks.size();
}

UINT NickList::Find(const tstring& sNick)
{
	for(UINT i = 0; i < m_vecNicks.size(); ++i)
	{
		if(_tcsicmp(sNick.c_str(), m_vecNicks[i]->nick.c_str()) == 0)
		{
			return i;
		}
	}
	return -1;
}

NickListEntry* NickList::GetEntry(UINT i)
{
	_ASSERTE(i < m_vecNicks.size());

	return m_vecNicks[i];
}

NickListEntry* NickList::GetEntry(const tstring& sNick)
{
	UINT i = Find(sNick);
	if(i != -1)
	{
		return m_vecNicks[i];
	}
	return NULL;
}
