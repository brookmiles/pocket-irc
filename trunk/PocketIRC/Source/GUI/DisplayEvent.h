#ifndef _DISPLAYEVENT_H_INCLUDED_
#define _DISPLAYEVENT_H_INCLUDED_

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

	void SetText(const tstring& sText) { m_sText = sText; }
	const tstring& GetText() const { return m_sText; }

	void SetKey(const tstring& sKey) { m_sKey = sKey; }
	const tstring& GetKey() const { return m_sKey; }

protected:

	int m_iHighlight;
	tstring m_sKey;
	tstring m_sText;
};

#endif//_DISPLAYEVENT_H_INCLUDED_