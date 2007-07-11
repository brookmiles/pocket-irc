#ifndef _NICKLIST_H_INCLUDED_
#define _NICKLIST_H_INCLUDED_

#include "StringT.h"
#include "Vector.h"

class NickListEntry
{
public:
	String nick;
	bool op;
	bool voice;
};

class NickList
{
public:
	NickList();
	~NickList();

	void AddNick(const String& sNick, bool bOp, bool bVoice);
	void RemoveNick(const String& sNick);
	void Clear();

	UINT Count();
	UINT Find(const String& sNick);
	NickListEntry* GetEntry(UINT i);
	NickListEntry* GetEntry(const String& sNick);

private:
	Vector<NickListEntry*> m_vecNicks;
};

#endif//_NICKLIST_H_INCLUDED_
