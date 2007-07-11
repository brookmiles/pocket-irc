#include "PocketIRC.h"
#include "IrcStringFormat.h"

static const COLORREF COLORS[] = {
	RGB(255, 255, 255),
	RGB(0, 0, 0),
	RGB(0, 0, 127),
	RGB(0, 127, 0),
	RGB(255, 0, 0),
	RGB(127, 0, 0),
	RGB(127, 0, 127),
	RGB(255, 127, 0),
	RGB(255, 255, 0),
	RGB(0, 255, 0),
	RGB(0, 127, 127),
	RGB(0, 255, 255),
	RGB(0, 0, 255),
	RGB(255, 0, 255),
	RGB(127, 127, 127),
	RGB(192, 192, 192),
};

COLORREF StringFormat::GetColorCode(UINT iColor)
{
	_ASSERTE(iColor < sizeof(COLORS)/sizeof(COLORS[0]));
	return COLORS[iColor];
}

UINT StringFormat::ParseColorCode(LPCTSTR psz, StringFormat& fmt)
{
	_ASSERTE(psz != NULL);

	UINT nUsed = 0;

	if(_istdigit(psz[0]))
	{
		nUsed = ParseSingleColor(&psz[0], &fmt.fg);
		if(nUsed && psz[nUsed] == ',' && _istdigit(psz[nUsed + 1]))
		{
			UINT nUsedBack = ParseSingleColor(&psz[nUsed + 1], &fmt.bg);
			if(nUsedBack)
				nUsed += nUsedBack + 1;
		}
	}

	return nUsed;
}

UINT StringFormat::ParseSingleColor(LPCTSTR psz, COLORREF* pcr)
{
	_ASSERTE(psz != NULL);
	_ASSERTE(pcr != NULL);

	_ASSERTE(_istdigit(psz[0]));

	UINT nUsed = 0;
	if(psz[0] == '0' || psz[0] == '1')
	{
		nUsed++;
		if(_istdigit(psz[1]))
		{
			nUsed++;
			UINT iPartColor = (psz[0] - '0') * 10;
			UINT iColor = (psz[1] - '0') + iPartColor;
			if(iColor >= 0 && iColor < 16)
			{
				*pcr = GetColorCode(iColor);
			}
		}
		else
		{
			UINT iColor = psz[0] - '0';
			*pcr = GetColorCode(iColor);
			nUsed = 1;
		}
	}
	else
	{
		nUsed++;
		UINT iColor = psz[0] - '0';
		*pcr = GetColorCode(iColor);
	}
	return nUsed;
}

UINT StringFormat::CollectFormats(const String& str, UINT iStart, const StringFormat& def, StringFormat& fmt)
{
	UINT nLen = str.Size();

	UINT iChar = iStart;
	// Collect formatting
	bool bFormatting = true;
	while(iChar < nLen && bFormatting)
	{
		switch(str[iChar])
		{
			case '\x02': // bold
				fmt.bold = !fmt.bold;
				iChar++;
			break;
			case '\x03': // color
				// If ParseColorCode doesn't find color codes, we reset colours to default
				fmt.fg = def.fg;
				fmt.bg = def.bg;
				iChar++;
				iChar += ParseColorCode(&str[iChar], fmt);
			break;
			case '\x0F': // Reset all formatting to default
				fmt = def;
				iChar++;
			break;
			case '\x16': // inverse
				fmt.inverse = !fmt.inverse;
				iChar++;
				break;
			case '\x1F': // underline
				fmt.underline = !fmt.underline;
				iChar++;
			break;
			default:
				bFormatting = false;
		}
	}
	return iChar - iStart;
}

UINT StringFormat::FindSegmentLen(const String& str, UINT iStart)
{
	UINT nLen = str.Size();
	UINT iSegEnd = iStart;
	while(iSegEnd < nLen)
	{
		if(str[iSegEnd] == '\x02' || str[iSegEnd] == '\x03' || str[iSegEnd] == '\x0F' || str[iSegEnd] == '\x16' || str[iSegEnd] == '\x1F')
			break;
		else
			iSegEnd++;
	}
	return iSegEnd - iStart;
}

String StringFormat::StripFormatting(const String& str)
{
	String out;
	out.Reserve(str.Capacity());

	UINT nLen = str.Size();
	UINT iChar = 0;
	while(iChar < nLen)
	{
		// Skip formatting
		StringFormat fmt;
		iChar += CollectFormats(str, iChar, fmt, fmt);

		//   Find length of string segment
		UINT iSegLen = FindSegmentLen(str, iChar);

		out += str.SubStr(iChar, iSegLen);
		iChar += iSegLen;
	}

	return out;
}

UINT StringFormat::FindWordBreak(LPCTSTR psz, UINT nFit)
{

	for(UINT i = nFit; (i > 0) && (nFit - i < POCKETIRC_WORDWRAP_BIAS); --i)
	{
		if(_istspace(psz[i - 1]))
		{
			return i;
		}
	}
	return nFit;
}
