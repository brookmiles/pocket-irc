#ifndef _DCCLISTWINDOW_H_INCLUDED_
#define _DCCLISTWINDOW_H_INCLUDED_

#include "GUI\Window.h"
#include "GUI\MainWindow.h"

#include "IDCCHandler.h"
#include "IDCCSession.h"

class DCCListWindow : 
	public Window,
	public ITabWindow
{
public:
	DECL_WINDOW_CLASSNAME("PocketIRCDCCListWnd");

	DCCListWindow();
	~DCCListWindow();

	HRESULT Create();

	int GetHighlight();
	void SetHighlight(int iHighlight);

	void Close();

	void SetMainWindow(MainWindow* pMainWindow);
	void SetDCCHandler(IDCCHandler* pDCCHandler);

	void AddSession(IDCCSession* pSession);
	void UpdateSession(IDCCSession* pSession);
	void RemoveSession(IDCCSession* pSession);

// ITabWindow
	HWND GetTabWindow(){ return GetWindow(); }
	const tstring GetTabTitle(){ return GetText(); }
	void OnOptionsChanged() {}
	void DoMenu(POINT pt);
	void DoDefaultAction() {}
	TABWINDOWTYPE GetTabType() { return TABWINDOWTYPE_DCC; };

protected:
	static BOOL CALLBACK DummyDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	void OnCommand(WPARAM wParam, LPARAM lParam);
	void OnNotify(WPARAM wParam, LPARAM lParam);
	void OnDestroy(WPARAM wParam, LPARAM lParam);
	void OnSize(WPARAM wParam, LPARAM lParam);
	void OnActivate(WPARAM wParam, LPARAM lParam);
	void OnInitMenuPopup(WPARAM wParam, LPARAM lParam);

	bool OnTabMenuCommand(UINT idCmd);

	void OnAccept();
	void OnClose(bool bAll);
	void OnClear();
	void OnSend();
	void OnChat();

	MainWindow* m_pMainWindow;
	IDCCHandler* m_pDCCHandler;

	int m_iHighlight;
	bool m_bActive;

	HMENU m_hMenu;
	HWND m_hDlg;
};

#endif//_DCCLISTWINDOW_H_INCLUDED_
