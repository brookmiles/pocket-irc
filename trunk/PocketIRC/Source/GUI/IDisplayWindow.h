#ifndef _IDISPLAYWINDOW_H_INCLUDED_
#define _IDISPLAYWINDOW_H_INCLUDED_

#include "Core\NetworkEvent.h"
#include "DisplayEvent.h"

class IDisplayWindow
{
public:
	virtual const tstring& GetKey() = 0;
	virtual void SetKey(const tstring& sTitle) = 0;

	virtual const tstring& GetTitle() = 0;
	virtual void SetTitle(const tstring& sTitle) = 0;

	virtual int GetHighlight() = 0;
	virtual void SetHighlight(int iHighlight) = 0;

	virtual void OnEvent(const NetworkEvent& networkEvent) = 0;
	virtual void PrintEvent(const DisplayEvent& displayEvent) = 0;
	virtual void Print(const tstring& sText) = 0;

	virtual void Close() = 0;

protected:
	~IDisplayWindow(){}
};

#endif//_IDISPLAYWINDOW_H_INCLUDED_
