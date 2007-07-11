#ifndef _DISPLAYEVENT_H_INCLUDED_
#define _DISPLAYEVENT_H_INCLUDED_

#include "StringT.h"

class DisplayEvent
{
public:
	typedef enum _HIGHLIGHT {
		HL_NONE,
		HL_MISC,
		HL_MESSAGE,
		HL_NOTIFY
	} HIGHLIGHT;

	DisplayEvent(){ m_iHighlight = HL_MISC; }

	int GetHighlight() const { return m_iHighlight; }
	void SetHighlight(int iHighlight) { m_iHighlight = iHighlight; }

	void SetText(const String& sText) { m_sText = sText; }
	const String& GetText() const { return m_sText; }

	void SetKey(const String& sKey) { m_sKey = sKey; }
	const String& GetKey() const { return m_sKey; }

protected:

	int m_iHighlight;
	String m_sKey;
	String m_sText;
};

#endif//_DISPLAYEVENT_H_INCLUDED_