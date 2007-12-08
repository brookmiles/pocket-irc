#include "PocketIRC.h"
#include "IrcString.h"

// These are pretty useless, "standard" my ass
//static const TCHAR validchars[] = _T("abcdefghijklmnopqsrtuvwxyzABCDEFGHIJKLMNOPQSRTUVWXYZ1234567890^[]{}\\|`_-@.");
//static const TCHAR validfirst[] = _T("abcdefghijklmnopqsrtuvwxyzABCDEFGHIJKLMNOPQSRTUVWXYZ^[]{}\\|`_");

int IrcStringGetWord(LPTSTR dst, LPCTSTR src, int iMaxLen, LPCTSTR* ppEnd)
{
	_ASSERTE(dst != NULL);
	_ASSERTE(src != NULL);
	_ASSERTE(iMaxLen >= 0);

	int i = 0;
	for(; src[i] != '\0' && !(src[i] == ' ') && i < iMaxLen; ++i)
	{
		dst[i] = src[i];
	}
	dst[i] = 0;

	if(ppEnd)
		*ppEnd = &src[i];

	return i;
}


// I have seen non-breaking spaces used in EFNet channels, so we can't arbitrarily skip whitespace.  
// IRC protocol specifies a single spaces as a parameter seperator
LPCTSTR IrcStringSkipSpaces(LPCTSTR psz)
{
	LPCTSTR seek = psz;
	while(*seek != '\0' && *seek == ' ')
		++seek;
	return seek;
}

bool IsNick(const tstring& sNick)
{
	return !IsChannel(sNick);
}


bool IsChannel(const tstring& sChannel)
{
	switch(sChannel[0])
	{
	case '#':
	case '&':
	case '+':
	case '!':
		return true;
	default:
		return false;
	}
}

bool IsUserString(const tstring& sUser)
{
	tstring::size_type c;
	return ((c = sUser.find_first_of('!')) != tstring::npos) && 
		((c = sUser.find_first_of('@', ++c)) != tstring::npos);
}

tstring GetPrefixNick(const tstring& sPrefix)
{
	if(sPrefix.size())
	{
		tstring::size_type c = sPrefix.find_first_of('!');
		if(c != tstring::npos)
		{
			return sPrefix.substr(0, c);
		}
		else
		{
			return sPrefix;
		}
	}
	return _T("");
}

tstring GetPrefixIdent(const tstring& sPrefix)
{
	if(sPrefix.size())
	{
		tstring::size_type start = sPrefix.find_first_of('!');
		if(start != tstring::npos)
		{
			++start;

			tstring::size_type end = sPrefix.find_first_of('@', start);
			if(end != tstring::npos)
			{
				return sPrefix.substr(start, end - start);
			}
		}
	}
	return _T("");
}

tstring GetPrefixHost(const tstring& sPrefix)
{
	if(sPrefix.size())
	{
		tstring::size_type start = sPrefix.find_first_of('!');
		if(start != tstring::npos)
		{
			start = sPrefix.find_first_of('@', ++start);
			if(start != tstring::npos)
			{
				return sPrefix.substr(++start);
			}
		}
	}
	return _T("");
}

tstring StripNick(const tstring& sNick, const tstring& sModeChars)
{
	tstring::size_type start = sNick.find_first_not_of(sModeChars);
	if(start != tstring::npos)
	{
		return sNick.substr(start);
	}
	return _T("");
}

bool IsUrl(const tstring& sUrl)
{
	tstring url = _T("http://");
	if(sUrl.compare(0, url.size(), url))
		return true;

	url = _T("www.");
	if(sUrl.compare(0, url.size(), url))
		return true;
	
	return false;
}

bool NickHasMode(const tstring& sNick, TCHAR mode)
{
	return (sNick.find_first_of(mode) != tstring::npos);
}
