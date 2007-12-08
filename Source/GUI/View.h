#ifndef _VIEW_H_INCLUDED_
#define _VIEW_H_INCLUDED_

#include "Window.h"
#include "Formatter.h"

struct ViewMsg
{
	tstring msg;
	UINT cy;
};

class View : public Window
{
public:
	DECL_WINDOW_CLASSNAME("PocketIRCViewWnd");

	View();
	~View();

	HRESULT Create(HWND hParent, UINT iID, int x, int y, int w, int h);

	void AddLine(const tstring& sText);
	void Update(bool bUpdateBounds = false);
	void ScrollIndex(UINT iScroll);

	void Clear();

	bool HitTestMsg(WORD x, WORD y, UINT* piMsgStart, UINT* piCharStart);
	tstring GetWordAtChar(UINT iMsg, UINT iChar);
	tstring GetWordAtPoint(POINT pt);

	tstring GetSelectedWord();

private:
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	void OnDestroy(WPARAM wParam, LPARAM lParam);
	void OnSize(WPARAM wParam, LPARAM lParam);
	void OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnEraseBkgnd(WPARAM wParam, LPARAM lParam);
	void OnVScroll(WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(WPARAM wParam, LPARAM lParam);
	void OnMouseMove(WPARAM wParam, LPARAM lParam);
	void OnCaptureChanged(WPARAM wParam, LPARAM lParam);

	void UpdateMsgBounds(ViewMsg& msg, HDC hdc, UINT cx);
	void UpdateScrollBar();
	void ClearBounds();

	void DrawUpdate(HDC hdc);
	UINT Draw(HDC hdc, const RECT& rect);
	UINT DrawMsg(ViewMsg& msg, HDC hdc, UINT cx, UINT dy);
	void DrawSelection(ViewMsg& msg, HDC hdc, UINT cx, UINT dy, UINT iMsg);

	bool StartSel(WORD xPos, WORD yPos);
	bool TrackSel(WORD xPos, WORD yPos);
	bool EndSel(bool bCopy);
	void CopySelection();

	struct SELECTION
	{
		bool valid;
		UINT iMsgStart;
		UINT iCharStart;
		UINT iMsgEnd;
		UINT iCharEnd;
	} m_sel;

	UINT m_iStart;
	std::vector<ViewMsg*> m_vecMsgs;
	Formatter m_Formatter;

	RECT m_rcOldClient;
};

#endif//_VIEW_H_INCLUDED_
