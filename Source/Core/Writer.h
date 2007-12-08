#ifndef _WRITER_H_INCLUDED_
#define _WRITER_H_INCLUDED_

#include "Network\ITransport.h"

#include "IWriteNetworkEvent.h"

class Writer : public IWriteNetworkEvent
{
public:
	Writer();
	~Writer();

	void SetTransport(ITransportWrite* pTransport);

// IWriteNetworkEvent
	void WriteEvent(const NetworkEvent& event);


protected:
	void Write(const tstring& sMsg);
	ITransportWrite* m_pTransport;
};

#endif//_WRITER_H_INCLUDED_
