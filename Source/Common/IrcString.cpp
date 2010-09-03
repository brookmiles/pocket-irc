#include "PocketIRC.h"
#include "IrcString.h"
#include "StringUtil.h"

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

bool IsChannelString(const tstring& sChannel)
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

tstring StripNickModes(const tstring& sNick, const tstring& sModeChars)
{
	tstring::size_type start = sNick.find_first_not_of(sModeChars);
	if(start != tstring::npos)
	{
		return sNick.substr(start);
	}
	return _T("");
}

tstring StripNickModesAndInvalidChars(const tstring& sNick, const tstring& sModeChars)
{
	tstring invalidChars = _T("<>,.()*!#$%&:;\"\'/?");
	tstring::size_type start = sNick.find_first_not_of(sModeChars + invalidChars);
	tstring::size_type end = sNick.find_last_not_of(sModeChars + invalidChars);

	if(start != tstring::npos)
	{
		end = end != tstring::npos ? end - start + 1 : tstring::npos;
		return sNick.substr(start, end);
	}
	return _T("");
}

bool NickHasMode(const tstring& sNick, TCHAR mode)
{
	return (sNick.find_first_of(mode) != tstring::npos);
}

bool IsUrl(const tstring& sUrl)
{
	TCHAR* urlPrefixes[] = {
		_T("http://"),
		_T("https://"),
		_T("www.")
	};

	for(int i = 0; i < sizeof(urlPrefixes)/sizeof(*urlPrefixes); ++i)
	{
		tstring pref = urlPrefixes[i];
		if(Compare(sUrl.substr(0, pref.size()), pref, false))
			return true;
	}
	return false;
}

