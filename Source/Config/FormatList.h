#ifndef _FORMATLIST_H_INCLUDED_
#define _FORMATLIST_H_INCLUDED_

#include "Format.h"
#include "Vector.h"

class FormatList
{
public:
	FormatList();
	FormatList(FormatList& from);
	~FormatList();

	const FormatList& operator=(FormatList& from);

	Format* GetFormat(int idEvent, bool bIncoming);
	void AddFormat(Format* pFormat);

	UINT Count();
	Format* Item(UINT nIndex);
	void Clear();

private:
	Vector<Format*> m_vecFormats;
};

#endif//_FORMATLIST_H_INCLUDED_
