#ifndef _IWRITENETWORKEVENT_H_INCLUDED_
#define _IWRITENETWORKEVENT_H_INCLUDED_

#include "NetworkEvent.h"

class IWriteNetworkEvent
{
public:
	virtual void WriteEvent(const NetworkEvent& event) = 0;
	virtual void Write(const tstring& sMsg) = 0;

protected:
	~IWriteNetworkEvent(){}
};

#endif//_IWRITENETWORKEVENT_H_INCLUDED_
