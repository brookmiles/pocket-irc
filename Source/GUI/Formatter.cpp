#include "PocketIRC.h"
#include "Formatter.h"

#define POCKETIRC_FORMATTER_INDENT 8

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

Formatter::Formatter()
{

}

Formatter::~Formatter()
{

}


/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

void Formatter::SetDefaultFmt(const StringFormat& fmt)
{
	m_fmt = fmt;
}

UINT Formatter::GetLineHeight(HDC hdc)
{
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	return tm.tmHeight;
}

// While not done
//   Collect formatting data
//   Find length of string segment
//   If wrapping
//     If longer than width
//       Stop at width, advance to next line
//   Apply formatting
//   Draw segment using current formatting
// End while

void Formatter::FormatOut(HDC hdc, const tstring& str, RECT& rc, DWORD dwFlags)
{
	UINT nLen = str.size();
	UINT cyUsed = 0;
	UINT cxUsed = 0;

	StringFormat fmt = m_fmt;

	UINT iChar = 0;
	while(iChar < nLen)
	{
		// Collect formatting
		iChar += StringFormat::CollectFormats(str, iChar, m_fmt, fmt);
		
		//   Find length of string segment
		UINT iSegEnd = iChar;
		iSegEnd += StringFormat::FindSegmentLen(str, iChar);

		//   If anything to draw
		if(iSegEnd - iChar > 0)
		{
			SIZE sz;

			UINT nDrawSeg = iSegEnd - iChar;

			if(dwFlags & FMT_WRAP)
			{
				UINT nFit = 0;
				BOOL bGetExtentOk = GetTextExtentExPoint(hdc, &str[iChar], nDrawSeg, 
					(rc.right - rc.left) - cxUsed, (int*)&nFit, NULL, &sz);
				_ASSERTE(bGetExtentOk);

				if(nFit < nDrawSeg)
				{
					nDrawSeg = StringFormat::FindWordBreak(&str[iChar], nFit);
				}
			}
			else
			{
				BOOL bGetExtentOk = GetTextExtentExPoint(hdc, &str[iChar], nDrawSeg, 
					0, NULL, NULL, &sz);
				_ASSERTE(bGetExtentOk);

				rc.right += sz.cx;
			}

			if(fmt.inverse)
			{
				SetTextColor(hdc, fmt.bg != -1 ? fmt.bg : (~fmt.fg & 0x00FFFFFF));
				SetBkColor(hdc, fmt.fg);
				SetBkMode(hdc, OPAQUE);
			}
			else
			{
				SetTextColor(hdc, fmt.fg);
				SetBkColor(hdc, fmt.bg);
				SetBkMode(hdc, (fmt.bg == -1) ? TRANSPARENT : OPAQUE);
			}

			if(!(dwFlags & FMT_CALC))
			{
				BOOL bExtOutOk = ExtTextOut(hdc, rc.left + cxUsed, rc.top + cyUsed, 0, NULL, &str[iChar], nDrawSeg, NULL);
				if(!bExtOutOk) 
				{
					_TRACE("ExtTextOut() FAILED: 0x%08X", GetLastError());
				}

				if(fmt.bold)
				{
					SetBkMode(hdc, TRANSPARENT);
					bExtOutOk = ExtTextOut(hdc, rc.left + cxUsed + 1, rc.top + cyUsed, 0, NULL, &str[iChar], nDrawSeg, NULL);
					if(!bExtOutOk) 
					{
						_TRACE("ExtTextOut() FAILED: 0x%08X", GetLastError());
					}
				}

				if(fmt.underline)
				{
					HPEN hp = CreatePen(PS_SOLID, DRA::SCALEY(1), GetTextColor(hdc));
					HPEN hpOld = (HPEN)SelectObject(hdc, hp);

					POINT pt[2] = { 
						{rc.left + cxUsed, rc.top + cyUsed + sz.cy - 1}, 
						{rc.left + cxUsed + sz.cx, rc.top + cyUsed + sz.cy - 1} 
					};

					Polyline(hdc, pt, 2);

					SelectObject(hdc, hpOld);
					DeleteObject(hp);
				}
			}			

			cxUsed += sz.cx;

			// Advance to next line if this one went beyond it
			if((dwFlags & FMT_WRAP) && nDrawSeg < iSegEnd - iChar)
			{
				cxUsed = POCKETIRC_FORMATTER_INDENT;
				cyUsed += sz.cy;
			}
			rc.bottom = rc.top + cyUsed + sz.cy;

			// Advance past drawn segment
			iChar += nDrawSeg;
		}
	}
}

UINT Formatter::HitTest(HDC hdc, const tstring& strFmt, RECT& rc, DWORD dwFlags, POINT pt)
{
	UINT cyUsed = 0;
	UINT cxUsed = 0;

	tstring str = StringFormat::StripFormatting(strFmt);
	UINT nLen = str.size();

	UINT iChar = 0;
	while(iChar < nLen)
	{
		//   Find length of string segment
		UINT nSegLen = nLen - iChar;
		UINT nLineFit = nSegLen;

		bool bLastLine = false;
		if(dwFlags & FMT_WRAP)
		{
			SIZE sz;
			BOOL bGetExtentOk = GetTextExtentExPoint(hdc, &str[iChar], nSegLen, 
				(rc.right - rc.left) - cxUsed, (int*)&nLineFit, NULL, &sz);
			_ASSERTE(bGetExtentOk);

			if(nLineFit < nSegLen)
			{
				nLineFit = StringFormat::FindWordBreak(&str[iChar], nLineFit);
			}

			if(rc.top + (long)cyUsed + (long)GetLineHeight(hdc) >= pt.y)
			{
				bLastLine = true;
				rc.right = pt.x;
			}
		}
		else if(!(dwFlags & FMT_WRAP))
		{
			bLastLine = true;
			rc.right = pt.x;
		}

		//   If anything to draw
		SIZE sz;

		UINT nDrawSeg = 0;
		BOOL bGetExtentOk = GetTextExtentExPoint(hdc, &str[iChar], nLineFit, 
			(rc.right - rc.left) - cxUsed, (int*)&nDrawSeg, NULL, &sz);
		_ASSERTE(bGetExtentOk);

		if(bLastLine)
		{
			return iChar + nDrawSeg;
		}

		cxUsed += sz.cx;

		// Advance to next line if this one went beyond it
		if(nLineFit < nSegLen)
		{
			cxUsed = POCKETIRC_FORMATTER_INDENT;
			cyUsed += sz.cy;
		}
		rc.bottom = rc.top + cyUsed + sz.cy;

		// Advance to next line
		iChar += nLineFit;
	}
	return iChar;
}

void Formatter::DrawSelection(HDC hdc, const tstring& strFmt, RECT& rc, DWORD dwFlags, UINT iSelStart, UINT iSelEnd)
{
	UINT cyUsed = 0;
	UINT cxUsed = 0;

	tstring str = StringFormat::StripFormatting(strFmt);
	UINT nLen = str.size();

	UINT iChar = 0;
	while(iChar < nLen && iChar < iSelEnd)
	{
		//   Find length of string segment
		UINT nSegLen = nLen - iChar;
		//   If anything to draw
		SIZE sz;

		// How many more chars fit on this line
		UINT nLineFit = nSegLen;
		if(dwFlags & FMT_WRAP)
		{
			BOOL bGetExtentOk = GetTextExtentExPoint(hdc, &str[iChar], nSegLen, 
				(rc.right - rc.left) - cxUsed, (int*)&nLineFit, NULL, &sz);
			_ASSERTE(bGetExtentOk);

			if(nLineFit < nSegLen)
			{
				nLineFit = StringFormat::FindWordBreak(&str[iChar], nLineFit);
			}
		}

		UINT nDrawSeg = nLineFit;
		if(iChar < iSelStart && iChar + nDrawSeg > iSelStart)
		{
			// Skip up to beginning of selection
			nDrawSeg = iSelStart - iChar;
		}
		else if(iChar >= iSelStart && iChar + nDrawSeg > iSelEnd)
		{
			// Draw up to end of selection
			nDrawSeg = iSelEnd - iChar;
		}

		if(iChar + nDrawSeg > nLen)
		{
			// Don't draw past end of string if sel end is beyond it
			nDrawSeg = nLen - iChar;
		}

		// Now get the final dimensions of what we're going to draw
		BOOL bGetExtentOk = GetTextExtentExPoint(hdc, &str[iChar], nDrawSeg, 
			0, NULL, NULL, &sz);
		_ASSERTE(bGetExtentOk);

		if(!(dwFlags & FMT_WRAP))
		{
			rc.right += sz.cx;
		}

		if(iChar >= iSelStart)
		{
			// Draw text in inverted default colors
			SetTextColor(hdc, RGB(255 - GetRValue(m_fmt.fg), 255 - GetGValue(m_fmt.fg), 255 - GetBValue(m_fmt.fg)));
			SetBkMode(hdc, OPAQUE);
			SetBkColor(hdc, m_fmt.fg);

			BOOL bExtOutOk = ExtTextOut(hdc, rc.left + cxUsed, rc.top + cyUsed, 0, NULL, &str[iChar], nDrawSeg, NULL);
			if(!bExtOutOk) 
			{
				_TRACE("ExtTextOut() FAILED: 0x%08X", GetLastError());
			}
		}

		cxUsed += sz.cx;

		// Advance to next line if this one went beyond it
		if((dwFlags & FMT_WRAP) && nDrawSeg == nLineFit)
		{
			cxUsed = POCKETIRC_FORMATTER_INDENT;
			cyUsed += sz.cy;
		}
		rc.bottom = rc.top + cyUsed + sz.cy;

		// Advance past drawn segment
		iChar += nDrawSeg;
	}
}
