#ifndef _IDCCCHAT_H_INCLUDED_
#define _IDCCCHAT_H_INCLUDED_

#include "StringT.h"

class IDCCChat
{
public:
	virtual void Say(const String& str) = 0;
	virtual void Act(const String& str) = 0;
	virtual void CloseChat() = 0;

	virtual String GetRemoteUser() = 0;
};

#endif//_IDCCCHAT_H_INCLUDED_
