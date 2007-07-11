#include "PocketIRC.h"
#include "IrcString.h"

static const TCHAR validchars[] = _T("abcdefghijklmnopqsrtuvwxyzABCDEFGHIJKLMNOPQSRTUVWXYZ1234567890^[]{}\\|`_-@.");
static const TCHAR validfirst[] = _T("abcdefghijklmnopqsrtuvwxyzABCDEFGHIJKLMNOPQSRTUVWXYZ^[]{}\\|`_");

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

//LPCTSTR StringSkipWhite(LPCTSTR psz)
//{
//	LPCTSTR seek = psz;
//	while(*seek != '\0' && _istspace(*seek))
//		++seek;
//	return seek;
//}

// I have seen non-breaking spaces used in EFNet channels, so we can't arbitrarily skip whitespace.  
// IRC protocol specifies a single spaces as a parameter seperator
LPCTSTR IrcStringSkipSpaces(LPCTSTR psz)
{
	LPCTSTR seek = psz;
	while(*seek != '\0' && *seek == ' ')
		++seek;
	return seek;
}

bool IsNick(const String& sNick)
{
	return sNick[0] != '\0' && _tcschr(validfirst, sNick[0]) != NULL && 
		_tcsspn(sNick.Str(), validchars) == sNick.Size();
}

bool IsChannel(const String& sChannel)
{
	return (sChannel[0] == '#' || sChannel[0] == '&' || sChannel[0] == '+' || 
		sChannel[0] == '!') && (_tcschr(sChannel.Str(), ' ') == NULL);
}

bool IsUserString(const String& sUser)
{
	TCHAR* pc;
	return ((pc = _tcschr(sUser.Str(), '!')) != NULL) && 
		(_tcschr(pc + 1, '@') != NULL);
}

String GetPrefixNick(const String& sPrefix)
{
	if(sPrefix.Str())
	{
		TCHAR* pNickEnd = _tcschr(sPrefix.Str(), '!');
		if(pNickEnd)
		{
			return String(sPrefix, pNickEnd - sPrefix.Str());
		}
		else
		{
			return sPrefix;
		}
	}
	return String(_T(""));
}

String GetPrefixIdent(const String& sPrefix)
{
	if(sPrefix.Str())
	{
		TCHAR* pIdentStart = _tcschr(sPrefix.Str(), '!');
		if(pIdentStart)
		{
			++pIdentStart;

			TCHAR* pIdentEnd = _tcschr(pIdentStart, '@');
			if(pIdentEnd)
			{
				return String(pIdentStart, pIdentEnd - pIdentStart);
			}
		}
	}
	return String(_T(""));
}

String GetPrefixHost(const String& sPrefix)
{
	if(sPrefix.Str())
	{
		TCHAR* pIdentStart = _tcschr(sPrefix.Str(), '!');
		if(pIdentStart)
		{
			++pIdentStart;
			TCHAR* pIdentEnd = _tcschr(pIdentStart, '@');
			if(pIdentEnd)
			{
				++pIdentEnd;
				return String(pIdentEnd);
			}
		}
	}
	return String(_T(""));
}

String StripNick(const String& sNick)
{
	UINT nCharStart = (UINT)_tcscspn(sNick.Str(), validfirst);

	if(nCharStart < sNick.Size())
	{
		UINT nChars = (UINT)_tcsspn(sNick.Str() + nCharStart, validchars);
		if(nChars > 0)
		{
			return sNick.SubStr(nCharStart, nChars);
		}
	}
	return _T("");
}

bool IsUrl(const String& sUrl)
{
	if(_tcsnicmp(_T("http://"), sUrl.Str(), 7) == 0)
		return true;
	else if(_tcsnicmp(_T("www."), sUrl.Str(), 4) == 0)
		return true;
	else
		return false;
}

bool NickHasMode(const String& sNick, TCHAR mode)
{
	UINT nCharStart = (UINT)_tcscspn(sNick.Str(), validfirst);
	if(nCharStart < sNick.Size())
	{
		String sModes = sNick.SubStr(0, nCharStart);
		TCHAR* chr = _tcschr(sModes.Str(), mode);
		return chr != NULL;
	}
	return false;
}
