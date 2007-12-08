#ifndef _IIDENTSERVER_H_INCLUDED_
#define _IIDENTSERVER_H_INCLUDED_

class IIdentServer
{
public:
	virtual HRESULT Start(USHORT usPort, const tstring& sUserName) = 0;
	virtual bool IsRunning() = 0;
	virtual void Stop() = 0;

protected:
	~IIdentServer(){}
};


#endif//_IIDENTSERVER_H_INCLUDED_
