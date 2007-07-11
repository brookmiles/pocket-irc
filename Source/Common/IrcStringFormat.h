#ifndef _IRCSTRINGFORMAT_H_INCLUDED_
#define _IRCSTRINGFORMAT_H_INCLUDED_

#include "StringT.h"

struct StringFormat
{
	static String StripFormatting(const String& str);
	static UINT CollectFormats(const String& str, UINT iStart, const StringFormat& def, StringFormat& fmt);
	static UINT FindSegmentLen(const String& str, UINT iStart);
	static UINT ParseColorCode(LPCTSTR psz, StringFormat& fmt);
	static UINT ParseSingleColor(LPCTSTR psz, COLORREF* pcr);
	static COLORREF GetColorCode(UINT iColor);
	static UINT FindWordBreak(LPCTSTR psz, UINT nFit);
	COLORREF fg;
	COLORREF bg;
	bool bold;
	bool underline;
	bool inverse;
};

#endif//_IRCSTRINGFORMAT_H_INCLUDED_
