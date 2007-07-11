#include "PocketIRC.h"
#include "SocketNotifyWindow.h"

ISocketNotify* ISocketNotify::Instance = NULL;

typedef struct _RECVNOTIFYINFO {
	IRecvNotifySink *pSink;
	int iRecv;
	LPARAM lParam;
}RECVNOTIFYINFO;

typedef struct _CONNECTNOTIFYINFO {
	IRecvNotifySink *pSink;
	HRESULT hr;
	LPARAM lParam;
}CONNECTNOTIFYINFO;

typedef struct _ACCEPTNOTIFYINFO {
	IAcceptNotifySink *pSink;
	SOCKET socket;
	LPARAM lParam;
}ACCEPTNOTIFYINFO;

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

SocketNotifyWindow::SocketNotifyWindow()
{
	_TRACE("SocketNotifyWindow(0x%08X)::SocketNotifyWindow()", this);
}

SocketNotifyWindow::~SocketNotifyWindow()
{
	_TRACE("SocketNotifyWindow(0x%08X)::~SocketNotifyWindow()", this);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Creation
/////////////////////////////////////////////////////////////////////////////

HRESULT SocketNotifyWindow::Create()
{
	_TRACE("SocketNotifyWindow(0x%08X)::Create()", this);
	
	HRESULT hr = Window::Create(NULL, _T("Boo."), WS_POPUP, 0, 0, 0, 0, 0);
	_ASSERTE(SUCCEEDED(hr));

	return hr;
}

bool SocketNotifyWindow::IsValidSink(LPARAM lParam)
{
	return m_vecValidSinks.Find(lParam) != Vector<LPARAM>::NPOS;
}

/////////////////////////////////////////////////////////////////////////////
//	IRecvNotify
/////////////////////////////////////////////////////////////////////////////

void SocketNotifyWindow::AddSink(LPARAM lParam)
{
	m_vecValidSinks.Append(lParam);
}

void SocketNotifyWindow::RemoveSink(LPARAM lParam)
{
	UINT index = m_vecValidSinks.Find(lParam);
	if(index != Vector<LPARAM>::NPOS)
	{
		m_vecValidSinks.Erase(index);
	}
}

bool SocketNotifyWindow::RecvNotify(IRecvNotifySink *pSink, int iRecv, LPARAM lParam)
{
	//_TRACE("SocketNotifyWindow(0x%08X)::RecvNotify(0x%08X, %i, 0x%08X)", this, pSink, iRecv, lParam);
	_ASSERTE(pSink != NULL);

	RECVNOTIFYINFO info;
	info.pSink = pSink;
	info.iRecv = iRecv;
	info.lParam = lParam;

	return SendMessage(m_hwnd, WM_USER_RECVNOTIFY, 0, (LPARAM)&info) != 0;
}

bool SocketNotifyWindow::ConnectNotify(IRecvNotifySink *pSink, HRESULT hr, LPARAM lParam)
{
	_TRACE("SocketNotifyWindow(0x%08X)::ConnectNotify(0x%08X, 0x%08X, 0x%08X)", this, pSink, hr, lParam);
	_ASSERTE(pSink != NULL);

	CONNECTNOTIFYINFO info;
	info.pSink = pSink;
	info.hr = hr;
	info.lParam = lParam;

	return SendMessage(m_hwnd, WM_USER_CONNECTNOTIFY, 0, (LPARAM)&info) != 0;
}

bool SocketNotifyWindow::AcceptNotify(IAcceptNotifySink *pSink, SOCKET socket, LPARAM lParam)
{
	_TRACE("SocketNotifyWindow(0x%08X)::AcceptNotify(0x%08X, 0x%08X, 0x%08X)", this, pSink, socket, lParam);
	_ASSERTE(pSink != NULL);

	ACCEPTNOTIFYINFO info;
	info.pSink = pSink;
	info.socket = socket;
	info.lParam = lParam;

	return SendMessage(m_hwnd, WM_USER_ACCEPTNOTIFY, 0, (LPARAM)&info) != 0;
}

/////////////////////////////////////////////////////////////////////////////
//	Window Procedure
/////////////////////////////////////////////////////////////////////////////

LRESULT SocketNotifyWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLEMSGRET(WM_USER_RECVNOTIFY, OnRecvNotify);
		HANDLEMSGRET(WM_USER_CONNECTNOTIFY, OnConnectNotify);
		HANDLEMSGRET(WM_USER_ACCEPTNOTIFY, OnAcceptNotify);
		default:
			return DefWindowProc(m_hwnd, msg, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//	Message Handling
/////////////////////////////////////////////////////////////////////////////

LRESULT SocketNotifyWindow::OnRecvNotify(WPARAM wParam, LPARAM lParam)
{
	RECVNOTIFYINFO* info = (RECVNOTIFYINFO*)lParam;
	_ASSERTE(info != NULL);

	_ASSERTE(info->pSink != NULL);
	if(IsValidSink((LPARAM)info->pSink))
	{
		return (LRESULT)info->pSink->OnRecvNotify(info->iRecv, info->lParam);
	}
	else
	{
		return 0;
	}
}

LRESULT SocketNotifyWindow::OnConnectNotify(WPARAM wParam, LPARAM lParam)
{
	CONNECTNOTIFYINFO* info = (CONNECTNOTIFYINFO*)lParam;
	_ASSERTE(info != NULL);

	_ASSERTE(info->pSink != NULL);
	if(IsValidSink((LPARAM)info->pSink))
	{
		return (LRESULT)info->pSink->OnConnectNotify(info->hr, info->lParam);
	}
	else
	{
		return 0;
	}
}

LRESULT SocketNotifyWindow::OnAcceptNotify(WPARAM wParam, LPARAM lParam)
{
	ACCEPTNOTIFYINFO* info = (ACCEPTNOTIFYINFO*)lParam;
	_ASSERTE(info != NULL);

	_ASSERTE(info->pSink != NULL);
	if(IsValidSink((LPARAM)info->pSink))
	{
		return (LRESULT)info->pSink->OnAcceptNotify(info->socket, info->lParam);
	}
	else
	{
		return 0;
	}
}
