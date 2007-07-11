#ifndef _WINDOW_H_INCLUDED_
#define _WINDOW_H_INCLUDED_

#include "StringT.h"


class Window
{
public:
	Window();
	virtual ~Window();

	virtual HRESULT Create(HWND hParent, const String& sTitle, DWORD dwStyles, UINT uID, int x, int y, int w, int h);
	virtual HWND GetWindow() { return m_hwnd; }

	virtual String GetText();

protected:
	virtual LPCTSTR GetClassName() = 0;

	static HRESULT RegisterClass(LPCTSTR pszClassName);
	static HRESULT UnRegisterClass(LPCTSTR pszClassName);

	static LRESULT CALLBACK WndProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam) = 0;

	virtual void OnDestroy(WPARAM wParam, LPARAM lParam);

	HWND m_hwnd;
};


#define DECL_WINDOW_CLASSNAME(name) \
	static LPCTSTR ClassName() { return _T(name); } \
	static HRESULT RegisterClass() { return Window::RegisterClass(ClassName()); } \
	static HRESULT UnRegisterClass() { return Window::UnRegisterClass(ClassName()); } \
	LPCTSTR GetClassName() { return ClassName(); } 

//#define HANDLEMSGRET(m, h) case m: return (LRESULT)h(wParam, lParam)
//#define HANDLEMSG(m, h) case m: h(wParam, lParam); break

#endif//_WINDOW_H_INCLUDED_
