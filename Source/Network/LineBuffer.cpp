#include "PocketIRC.h"
#include "LineBuffer.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

LineBuffer::LineBuffer()
{
	m_nUsed = 0;
}

/////////////////////////////////////////////////////////////////////////////
//	ITransportReader Interface
/////////////////////////////////////////////////////////////////////////////

void LineBuffer::OnRead(BYTE *pData, UINT nSize)
{
	_ASSERTE(pData != NULL);

	for(UINT i = 0; i < nSize; ++i)
	{
		switch(pData[i])
		{
			case '\0':
				// wtf?
				//_ASSERTE(pData[i] != 0);
				OnError(E_UNEXPECTED);
			break;
			case '\r':
				// ignore
			break;
			case '\n':
			{
				USES_CONVERSION;

				m_szIRCLineBuffer[m_nUsed] = 0;
				OnLineRead(A2CT((char*)m_szIRCLineBuffer));
				m_nUsed = 0;
			}
			break;
			default:
				m_szIRCLineBuffer[m_nUsed] = pData[i];
				m_nUsed++;
		}

		if(m_nUsed >= sizeof(m_szIRCLineBuffer))
		{
			_ASSERTE(m_nUsed >= sizeof(m_szIRCLineBuffer));
			OnError(E_UNEXPECTED);

			m_nUsed = 0;
		}
	}
}
