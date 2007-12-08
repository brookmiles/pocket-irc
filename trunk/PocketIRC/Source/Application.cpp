#include "PocketIRC.h"
#include "Application.h"

#include "GUI\StatusWindow.h"
#include "GUI\ChannelWindow.h"
#include "GUI\QueryWindow.h"

#include "DCC\DCCListWindow.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

Application::Application()
{
	_TRACE("Application(0x%08X)::Application()", this);

	m_bInitOk = false;
	m_hInstance = NULL;
}

Application::~Application()
{
	_TRACE("Application(0x%08X)::~Application()", this);

	if(m_bInitOk)
	{
		// o helo
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Initialisation
/////////////////////////////////////////////////////////////////////////////

HRESULT Application::Init(HINSTANCE hInstance, LPCTSTR lpCmdLine, int nCmdLine)
{
	_TRACE("Application(0x%08X)::Init(0x%08X, 0x%08X, 0x%08X)", this, hInstance, lpCmdLine, nCmdLine);
	_ASSERTE(hInstance != NULL);
	_ASSERTE(lpCmdLine != NULL);
	
	m_hInstance = hInstance;

	InitCommonControls();
	srand(GetTickCount());
	
	HRESULT hr = InitWindows();
	_ASSERTE(SUCCEEDED(hr));

	hr = SocketTransport::Startup();
	_ASSERTE(SUCCEEDED(hr));

	if(SUCCEEDED(hr))
	{
		m_bInitOk = true;
	}
	
	return hr;
}

HRESULT Application::UnInit()
{
	_TRACE("Application(0x%08X)::UnInit()", this);
	_ASSERTE(m_bInitOk == true);
	
	SocketTransport::Cleanup();

	return UnInitWindows();
}

/////////////////////////////////////////////////////////////////////////////
//	Execution
/////////////////////////////////////////////////////////////////////////////

HRESULT Application::InitWindows()
{
	_TRACE("Application(0x%08X)::InitWindows()", this);

	HRESULT hr = MainWindow::RegisterClass();
	_ASSERTE(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	hr = SocketNotifyWindow::RegisterClass();
	_ASSERTE(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	hr = StatusWindow::RegisterClass();
	_ASSERTE(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	hr = ChannelWindow::RegisterClass();
	_ASSERTE(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	hr = QueryWindow::RegisterClass();
	_ASSERTE(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	hr = View::RegisterClass();
	_ASSERTE(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	hr = DCCListWindow::RegisterClass();
	_ASSERTE(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	hr = DCCChatWindow::RegisterClass();
	_ASSERTE(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	hr = wnd.Init();
	_ASSERTE(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	hr = wnd.Create();
	_ASSERTE(SUCCEEDED(hr));
	if(FAILED(hr))
		return hr;

	hr = notify.Create();
	_ASSERTE(SUCCEEDED(hr));

	return hr;
}

HRESULT Application::UnInitWindows()
{
	_TRACE("Application(0x%08X)::UnInitWindows()", this);
	_ASSERTE(m_bInitOk == true);

	DestroyWindow(notify.GetWindow());

	HRESULT hr = wnd.UnInit();
	_ASSERTE(SUCCEEDED(hr));

	hr = MainWindow::UnRegisterClass();
	_ASSERTE(SUCCEEDED(hr));
	
	hr = SocketNotifyWindow::UnRegisterClass();
	_ASSERTE(SUCCEEDED(hr));

	hr = DCCListWindow::UnRegisterClass();
	_ASSERTE(SUCCEEDED(hr));

	hr = DCCChatWindow::UnRegisterClass();
	_ASSERTE(SUCCEEDED(hr));

	hr = View::UnRegisterClass();
	_ASSERTE(SUCCEEDED(hr));

	hr = StatusWindow::UnRegisterClass();
	_ASSERTE(SUCCEEDED(hr));

	hr = ChannelWindow::UnRegisterClass();
	_ASSERTE(SUCCEEDED(hr));

	hr = QueryWindow::UnRegisterClass();
	_ASSERTE(SUCCEEDED(hr));

	return hr;
}

HRESULT Application::Run()
{
	_TRACE("Application(0x%08X)::Run()", this);
	_ASSERTE(m_bInitOk == true);

	// Connect the dots

	ISocketNotify::Instance = &notify;

	transport.SetReader(&reader);
	transport.SetNotify(&notify);

	writer.SetTransport(&transport);
	reader.SetNotifySink(&session);

	session.SetWriter(&writer);
	session.AddEventHandler(&dcc);
	session.AddEventHandler(&wnd);

	ident.SetEventNotify(&wnd);
	ident.SetSocketNotify(&notify);

	wnd.SetSession(&session);
	wnd.SetTransport(&transport);
	wnd.SetIdentServer(&ident);
	wnd.SetDCCHandler(&dcc);

	dcc.SetMainWindow(&wnd);
	dcc.SetSession(&session);
	dcc.SetSocketNotify(&notify);

	SetForegroundWindow(wnd.GetWindow());

	// Go!
	HRESULT hr = RunMessageLoop();
	_ASSERTE(SUCCEEDED(hr));

	return hr;
}

HRESULT Application::RunMessageLoop()
{
	_TRACE("Application(0x%08X)::RunMessageLoop()", this);

	MSG msg;
	
	msg.wParam = E_UNEXPECTED;

	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return HRESULT_FROM_WIN32(msg.wParam);
}
