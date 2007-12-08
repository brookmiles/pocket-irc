#ifndef _NICKLIST_H_INCLUDED_
#define _NICKLIST_H_INCLUDED_

class NickListEntry
{
public:
	tstring nick;
	bool op;
	bool voice;
};

class NickList
{
public:
	NickList();
	~NickList();

	void AddNick(const tstring& sNick, bool bOp, bool bVoice);
	void RemoveNick(const tstring& sNick);
	void Clear();

	UINT Count();
	UINT Find(const tstring& sNick);
	NickListEntry* GetEntry(UINT i);
	NickListEntry* GetEntry(const tstring& sNick);

private:
	std::vector<NickListEntry*> m_vecNicks;
};

#endif//_NICKLIST_H_INCLUDED_
