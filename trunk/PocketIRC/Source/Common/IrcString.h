#ifndef _IRCSTRING_H_INCLUDED_
#define _IRCSTRING_H_INCLUDED_

#include "StringT.h"
#include "Vector.h"

int IrcStringGetWord(LPTSTR dst, LPCTSTR src, int iMaxLen, LPCTSTR* ppEnd);
LPCTSTR IrcStringSkipSpaces(LPCTSTR psz);

bool IsNick(const String& sNick);
bool IsChannel(const String& sChannel);
bool IsUserString(const String& sUser);
String GetPrefixNick(const String& sPrefix);
String GetPrefixHost(const String& sPrefix);
String GetPrefixIdent(const String& sPrefix);
String StripNick(const String& sNick);
bool NickHasMode(const String& sNick, TCHAR mode);
bool IsUrl(const String& sUrl);

#endif//_IRCSTRING_H_INCLUDED_
