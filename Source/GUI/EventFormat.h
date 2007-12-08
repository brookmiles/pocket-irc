#ifndef _EVENTFORMAT_H_INCLUDED_
#define _EVENTFORMAT_H_INCLUDED_

#include "DisplayEvent.h"
#include "Core\NetworkEvent.h"

void DisplayEventFormat(DisplayEvent& display, const NetworkEvent& network, const tstring& sMe);
tstring EventFormat(const NetworkEvent& network, const tstring& sMe, const tstring& sFmt);
tstring GetEventKey(const NetworkEvent& network);

#endif//_EVENTFORMAT_H_INCLUDED_