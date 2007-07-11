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

void NickList::AddNick(const String& sNick, bool bOp, bool bVoice)
{
	NickListEntry* pEntry = GetEntry(sNick);

	if(pEntry == NULL)
	{
		pEntry = new NickListEntry;
		pEntry->nick = sNick;

		m_vecNicks.Append(pEntry);
	}

	_ASSERTE(pEntry != NULL);

	pEntry->op = bOp;
	pEntry->voice = bVoice;
}

void NickList::RemoveNick(const String& sNick)
{
	UINT i = Find(sNick);
	if(i != -1)
	{
		delete m_vecNicks[i];
		m_vecNicks.Erase(i);
	}
}

void NickList::Clear()
{
	for(UINT i = 0; i < m_vecNicks.Size(); ++i)
	{
		delete m_vecNicks[i];
	}
	m_vecNicks.Clear();
}


UINT NickList::Count()
{
	return m_vecNicks.Size();
}

UINT NickList::Find(const String& sNick)
{
	for(UINT i = 0; i < m_vecNicks.Size(); ++i)
	{
		if(sNick.Compare(m_vecNicks[i]->nick, false))
		{
			return i;
		}
	}
	return -1;
}

NickListEntry* NickList::GetEntry(UINT i)
{
	_ASSERTE(i < m_vecNicks.Size());

	return m_vecNicks[i];
}

NickListEntry* NickList::GetEntry(const String& sNick)
{
	UINT i = Find(sNick);
	if(i != -1)
	{
		return m_vecNicks[i];
	}
	return NULL;
}
