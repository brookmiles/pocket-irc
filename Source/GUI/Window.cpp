#include "PocketIRC.h"
#include "Window.h"

#include "WindowString.h"

/////////////////////////////////////////////////////////////////////////////
// Static Initialisation and Cleanup
/////////////////////////////////////////////////////////////////////////////

HRESULT Window::RegisterClass(LPCTSTR pszClassName)
{
	_TRACE("Window::RegisterClass(\"%s\")", pszClassName);
		
	WNDCLASS wc = {sizeof(wc)};

	wc.style		 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	 = Window::WndProcStub;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = GetModuleHandle(NULL);
	wc.hIcon		 = NULL;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = pszClassName;

	bool bRegisterOk = (::RegisterClass(&wc) != 0);
	_ASSERTE(bRegisterOk);

	return bRegisterOk ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT Window::UnRegisterClass(LPCTSTR pszClassName)
{
	_TRACE("Window::UnRegisterClass(\"%s\")", pszClassName);

	BOOL bUnRegOk = UnregisterClass(pszClassName, GetModuleHandle(NULL));
	_ASSERTE(bUnRegOk);
	
	return bUnRegOk ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

/////////////////////////////////////////////////////////////////////////////
//	Stub Window Procedure
/////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Window::WndProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* pWnd = (Window*)GetWindowLong(hwnd, GWL_USERDATA);
	if(pWnd)
	{
		return pWnd->WndProc(msg, wParam, lParam);
	}
	else
	{
		_TRACE("Window::WndProcStub(0x%08X, 0x%08X, 0x%08X, 0x%08X)", hwnd, msg, wParam, lParam);
		if(msg == WM_CREATE)
		{
			_TRACE("...WM_CREATE");

			CREATESTRUCT* lpcs = (CREATESTRUCT*)lParam;
			pWnd = (Window*)lpcs->lpCreateParams;

			_ASSERTE(pWnd != NULL);

			pWnd->m_hwnd = hwnd;
			SetWindowLong(hwnd, GWL_USERDATA, (LONG)pWnd);

			return pWnd->WndProc(msg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}	
}

/////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
/////////////////////////////////////////////////////////////////////////////

Window::Window()
{
	_TRACE("Window(0x%08X)::Window()", this);
	m_hwnd = NULL;
}

Window::~Window()
{
	_TRACE("Window(0x%08X)::~Window()", this);
	if(IsWindow(m_hwnd))
	{
		DestroyWindow(m_hwnd);
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Window Creation
/////////////////////////////////////////////////////////////////////////////

HRESULT Window::Create(HWND hParent, const tstring& sTitle, DWORD dwStyles, UINT uID, int x, int y, int w, int h)
{
	_TRACE("Window(0x%08X)::Create(0x%08X, \"%s\", 0x%08X, %u) \"%s\"", this, hParent, sTitle.c_str(), dwStyles, uID, GetClassName());

	_ASSERTE(IsWindow(hParent) || hParent == NULL);

	HRESULT hr = E_UNEXPECTED;

	HWND hwnd = CreateWindow(GetClassName(), sTitle.c_str(), dwStyles, 
		x, y, w, h, hParent, (HMENU)uID, GetModuleHandle(NULL), (void*)this);

	_ASSERTE(IsWindow(hwnd));
	_ASSERTE(m_hwnd == hwnd);
	
	if(!IsWindow(hwnd))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		hr = S_OK;
	}
		
	return hr;
}

tstring Window::GetText()
{
	int len = GetWindowTextLength(m_hwnd);

	if(len > 0)
	{
		tstring sTitle;

		GetWindowString(m_hwnd, sTitle);
		return sTitle;
	}
	
	return _T("");
}

/////////////////////////////////////////////////////////////////////////////
//	Message Handling
/////////////////////////////////////////////////////////////////////////////

void Window::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	m_hwnd = NULL;
}
