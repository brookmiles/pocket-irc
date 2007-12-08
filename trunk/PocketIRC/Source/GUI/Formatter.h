#ifndef _FORMATTER_H_INCLUDED_
#define _FORMATTER_H_INCLUDED_

#include "IrcStringFormat.h"

class Formatter
{
public:
	enum FORMATTER_FLAGS { FMT_WRAP = 0x01, FMT_CALC = 0x02 };

	Formatter();
	~Formatter();

	void SetDefaultFmt(const StringFormat& fmt);

	// If FMT_WRAP, accept width in prc and return height in prc
	// Otherwise return output width and height in prc
	void FormatOut(HDC hdc, const tstring& str, RECT& rc, DWORD dwFlags);
	UINT HitTest(HDC hdc, const tstring& str, RECT& rc, DWORD dwFlags, POINT pt);
	void DrawSelection(HDC hdc, const tstring& strFmt, RECT& rc, DWORD dwFlags, UINT iSelStart, UINT iSelEnd);
	UINT GetLineHeight(HDC hdc);

private:

	StringFormat m_fmt;
};


#endif//_FORMATTER_H_INCLUDED_
