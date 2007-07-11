#ifndef _IDCCHANDLER_H_INCLUDED_
#define _IDCCHANDLER_H_INCLUDED_

#include "IDCCSession.h"


class IDCCHandler
{
public:
	virtual void ShowDCCListWindow(bool bShow) = 0;
	virtual bool IsDCCListWindowVisible() = 0;

	virtual UINT GetSessionCount() = 0;
	virtual IDCCSession* GetSession(UINT i) = 0;

	virtual String GetNick() = 0;

	virtual void RemoveSession(IDCCSession *pSession) = 0;

	virtual void Chat(const String& sUser) = 0;
	virtual void Send(const String& sUser) = 0;
};

#endif//_IDCCHANDLER_H_INCLUDED_
