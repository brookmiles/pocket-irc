#ifndef _IRCSTRINGFORMAT_H_INCLUDED_
#define _IRCSTRINGFORMAT_H_INCLUDED_

struct StringFormat
{
	static tstring StripFormatting(const tstring& str);
	static UINT CollectFormats(const tstring& str, UINT iStart, const StringFormat& def, StringFormat& fmt);
	static UINT FindSegmentLen(const tstring& str, UINT iStart);
	static UINT ParseColorCode(const tstring& str, StringFormat& fmt);
	static UINT ParseSingleColor(const tstring& str, COLORREF* pcr);
	static COLORREF GetColorCode(UINT iColor);
	static UINT FindWordBreak(const tstring& str, UINT nFit);
	COLORREF fg;
	COLORREF bg;
	bool bold;
	bool underline;
	bool inverse;
};

#endif//_IRCSTRINGFORMAT_H_INCLUDED_
