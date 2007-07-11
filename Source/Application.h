#ifndef _APPLICATION_H_INCLUDED_
#define _APPLICATION_H_INCLUDED_

#include "Network\SocketTransport.h"
#include "Network\SocketNotifyWindow.h"
#include "Core\Writer.h"
#include "Core\Reader.h"
#include "Core\Session.h"
#include "Core\IdentServer.h"
#include "GUI\MainWindow.h"
#include "DCC\DCCHandler.h"

class Application
{
public:
	Application();
	~Application();

	HRESULT Init(HINSTANCE hInstance, LPCTSTR lpCmdLine, int nCmdLine);
	HRESULT Run();
	HRESULT UnInit();
	
private:
	HRESULT InitWindows();
	HRESULT UnInitWindows();
	HRESULT RunMessageLoop();

	HINSTANCE m_hInstance;
	bool m_bInitOk;

	MainWindow wnd;
	SocketNotifyWindow notify;
	SocketTransport transport;
	Session session;
	Writer writer;
	Reader reader;
	IdentServer ident;
	DCCHandler dcc;
};

#endif//_APPLICATION_H_INCLUDED_
