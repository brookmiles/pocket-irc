#ifndef _IRCSTRING_H_INCLUDED_
#define _IRCSTRING_H_INCLUDED_

int IrcStringGetWord(LPTSTR dst, LPCTSTR src, int iMaxLen, LPCTSTR* ppEnd);
LPCTSTR IrcStringSkipSpaces(LPCTSTR psz);

bool IsChannelString(const tstring& sChannel);
tstring GetPrefixNick(const tstring& sPrefix);
tstring GetPrefixHost(const tstring& sPrefix);
tstring GetPrefixIdent(const tstring& sPrefix);
tstring StripNickModes(const tstring& sNick, const tstring& sModeChars);
tstring StripNickModesAndInvalidChars(const tstring& sNick, const tstring& sModeChars);
bool NickHasMode(const tstring& sNick, TCHAR mode);
bool IsUrl(const tstring& sUrl);

#endif//_IRCSTRING_H_INCLUDED_
