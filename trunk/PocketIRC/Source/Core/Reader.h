#ifndef _IRCREADER_H_INCLUDED_
#define _IRCREADER_H_INCLUDED_

#include "Network\ITransport.h"
#include "Network\LineBuffer.h"

#include "NetworkEvent.h"

// LineBuffer derives from and partially implements ITransportReader
class Reader :
	public LineBuffer
{
public:
	static bool ParseEvent(NetworkEvent& event, LPCTSTR pszLine);

	Reader();
	~Reader();

	void SetNotifySink(INetworkEventNotify* pNotify);

// ITransportReader
	void OnConnect(HRESULT hr, LPCTSTR pszError);
	void OnError(HRESULT hr);
	void OnClose();
// LineBuffer implements ITransportReader::OnRead()
// LineBuffer
	void OnLineRead(LPCTSTR pszLine);

private:
	INetworkEventNotify* m_pNotify;
};

#endif//_IRCREADER_H_INCLUDED_
