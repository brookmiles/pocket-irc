#ifndef _EVENTFORMAT_H_INCLUDED_
#define _EVENTFORMAT_H_INCLUDED_

#include "StringT.h"

#include "DisplayEvent.h"
#include "Core\NetworkEvent.h"

void DisplayEventFormat(DisplayEvent& display, const NetworkEvent& network, const String& sMe);
String EventFormat(const NetworkEvent& network, const String& sMe, const String& sFmt);
String GetEventKey(const NetworkEvent& network);

#endif//_EVENTFORMAT_H_INCLUDED_