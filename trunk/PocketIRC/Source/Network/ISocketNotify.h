#ifndef _ISOCKETNOTIFY_H_INCLUDED_
#define _ISOCKETNOTIFY_H_INCLUDED_

class IRecvNotifySink
{
public:
	virtual bool OnConnectNotify(HRESULT hr, LPARAM lParam) = 0;
	virtual bool OnRecvNotify(int iRecv, LPARAM lParam) = 0;
};

class IAcceptNotifySink
{
public:
	virtual bool OnAcceptNotify(SOCKET socket, LPARAM lParam) = 0;
};

class ISocketNotify
{
public:
	static ISocketNotify* Instance;

	virtual void AddSink(LPARAM lParam) = 0;
	virtual void RemoveSink(LPARAM lParam) = 0;

	virtual bool RecvNotify(IRecvNotifySink *pSink, int iRecv, LPARAM lParam) = 0;
	virtual bool ConnectNotify(IRecvNotifySink *pSink, HRESULT hr, LPARAM lParam) = 0;
	virtual bool AcceptNotify(IAcceptNotifySink *pSink, SOCKET socket, LPARAM lParam) = 0;
};

#endif//_ISOCKETNOTIFY_H_INCLUDED_
