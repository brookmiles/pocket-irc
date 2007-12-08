#ifndef _STRINGUTIL_H_INCLUDED_
#define _STRINGUTIL_H_INCLUDED_

void Split(const tstring& str, std::vector<tstring>& out, TCHAR sep, bool includeEmpty);
tstring GetWord(const tstring& str, unsigned index, bool getRest = false);

inline bool Compare(const tstring& one, const tstring& two, bool caseSensitive)
{
	return caseSensitive ? (one == two) : (_tcsicmp(one.c_str(), two.c_str()) == 0);
}

#endif//_STRINGUTIL_H_INCLUDED_
