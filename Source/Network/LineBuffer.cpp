#include "PocketIRC.h"
#include "Config/Options.h"
#include "LineBuffer.h"
#include "StringConv.h"

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
				m_szIRCLineBuffer[m_nUsed] = 0;

				// TODO really shouldn't be reading global options from here :/
				tstring tstr = g_Options.GetUTF8() ? UTF8ToTCHAR((char*)m_szIRCLineBuffer) : MBToTCHAR((char*)m_szIRCLineBuffer);
				OnLineRead(tstr.c_str());
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
