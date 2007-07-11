#ifndef _LINEBUFFER_H_INCLUDED_
#define _LINEBUFFER_H_INCLUDED_

#include "ITransport.h"


class LineBuffer : 
	public ITransportReader
{
public:
	LineBuffer();

	void OnRead(BYTE *pData, UINT nSize);

	virtual void OnLineRead(LPCTSTR pszLine) = 0;
private:
	BYTE m_szIRCLineBuffer[POCKETIRC_MAX_IRC_LINE_LEN + 1];
	UINT m_nUsed;
};


#endif//_LINEBUFFER_H_INCLUDED_
