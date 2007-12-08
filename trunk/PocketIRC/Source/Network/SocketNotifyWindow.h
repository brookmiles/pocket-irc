#ifndef _SOCKETNOTIFYWINDOW_H_INCLUDED_
#define _SOCKETNOTIFYWINDOW_H_INCLUDED_

#include "GUI\Window.h"
#include "SocketTransport.h"

#include "ISocketNotify.h"

class SocketNotifyWindow : 
	public Window,
	public ISocketNotify
{
	enum SOCKETNOTIFY_WM { 
		WM_USER_RECVNOTIFY = (WM_USER+1), 
		WM_USER_CONNECTNOTIFY, 
		WM_USER_ACCEPTNOTIFY 
	};
public:
	DECL_WINDOW_CLASSNAME("PocketIRCSocketNotifyWnd");

	SocketNotifyWindow();
	~SocketNotifyWindow();

// ISocketNotify
	void AddSink(LPARAM lParam);
	void RemoveSink(LPARAM lParam);
	bool RecvNotify(IRecvNotifySink *pSink, int iRecv, LPARAM lParam);
	bool ConnectNotify(IRecvNotifySink *pSink, HRESULT hr, LPARAM lParam);
	bool AcceptNotify(IAcceptNotifySink *pSink, SOCKET socket, LPARAM lParam);

	HRESULT Create();

private:
	bool IsValidSink(LPARAM lParam);

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRecvNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnConnectNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnAcceptNotify(WPARAM wParam, LPARAM lParam);

	std::vector<LPARAM> m_vecValidSinks;
};	

#endif//_SOCKETNOTIFYWINDOW_H_INCLUDED_
