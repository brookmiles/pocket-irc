#ifndef _WRITER_H_INCLUDED_
#define _WRITER_H_INCLUDED_

#include "StringT.h"

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
	void Raw(const String& sMsg);
	ITransportWrite* m_pTransport;
};

#endif//_WRITER_H_INCLUDED_
