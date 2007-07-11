#ifndef _TABSTRIP_H_INCLUDED_
#define _TABSTRIP_H_INCLUDED_

#include "Window.h"
#include "Vector.h"
#include "StringT.h"

//#define TABSTRIP_DEF_HEIGHT	22
#define TABSTRIP_MAX_TEXT_LEN 32

#define TABSTRIP_BUTTON_PADDING	7
#define TABSTRIP_SCROLL_WIDTH	10

class TabStrip : public Window
{
	class TabStripItem
	{
	public:
		String sText;
		HWND hActivate;
		LPARAM lParam;
		int iWidth;
		COLORREF crText;
	};

public:
	DECL_WINDOW_CLASSNAME("PocketIRCTabStrip");

	TabStrip();
	~TabStrip();

	void Update();
	bool UpdateSelection();

	UINT AddTab(const String& sTitle, HWND hActivate, LPARAM lParam);
	bool RemoveTab(UINT uIndex);
	bool FindTab(LPARAM lParam, UINT* pResult);

	bool GetTabParam(UINT uIndex, LPARAM* pParam);
	bool SetTabColor(UINT uIndex, COLORREF color, bool bRedraw);
	bool SetTabText(UINT uIndex, const String& sText, bool bRedraw);

	bool GetCurSel(UINT* pResult);
	void SetCurSel(UINT uIndex);

	void SetFont(HFONT hf, bool bUpdate);
	HFONT GetFont();

	int CalcTabHeight();

private:
	void UpdateScrollButtons();
	TabStripItem* GetTabItem(UINT uIndex);
	int CalcItemWidth(TabStripItem* pItem);

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	void OnPaint(WPARAM wParam, LPARAM lParam);
	void OnEraseBkgnd(WPARAM wParam, LPARAM lParam);
	void OnSize(WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(WPARAM wParam, LPARAM lParam);
	void OnCaptureChanged(WPARAM wParam, LPARAM lParam);

	Vector<TabStripItem*> m_vecItems;

	UINT m_uSelected;
	UINT m_uStart;
	HFONT m_hf;
	bool m_bScrollLeftVisible;
	bool m_bScrollLeftDown;
	bool m_bScrollRightVisible;
	bool m_bScrollRightDown;

	HWND m_hLastActive;
};

#endif//_TABSTRIP_H_INCLUDED_