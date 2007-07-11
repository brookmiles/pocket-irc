#include "PocketIRC.h"
#include "TabStrip.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

TabStrip::TabStrip()
{
	_TRACE("TabStrip(0x%08X)::TabStrip()", this);
	
	m_uSelected = 0;
	m_uStart = 0;
	m_hf = NULL;
	m_bScrollLeftVisible = false;
	m_bScrollLeftDown = false;
	m_bScrollRightVisible = false;
	m_bScrollRightDown = false;

	m_hLastActive = NULL;
}

TabStrip::~TabStrip()
{
	_TRACE("TabStrip(0x%08X)::~TabStrip()", this);

	for(UINT i = 0; i < m_vecItems.Size(); ++i)
	{
		delete m_vecItems[i];
	}
	m_vecItems.Shrink(0);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Procedure
/////////////////////////////////////////////////////////////////////////////

LRESULT TabStrip::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLEMSGRET(WM_CREATE, OnCreate);
		HANDLEMSG(WM_DESTROY, OnDestroy);
		HANDLEMSG(WM_PAINT, OnPaint);
		HANDLEMSG(WM_ERASEBKGND, OnEraseBkgnd);
		HANDLEMSG(WM_SIZE, OnSize);
		HANDLEMSG(WM_LBUTTONDOWN, OnLButtonDown);
		HANDLEMSG(WM_LBUTTONUP, OnLButtonUp);
		HANDLEMSG(WM_CAPTURECHANGED, OnCaptureChanged);
		default:
			return DefWindowProc(m_hwnd, msg, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Message Handling
/////////////////////////////////////////////////////////////////////////////

LRESULT TabStrip::OnCreate(WPARAM wParam, LPARAM lParam)
{
	SetWindowLong(m_hwnd, GWL_EXSTYLE, (GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_NOACTIVATE));
	return 0;
}

void TabStrip::OnPaint(WPARAM wParam, LPARAM lParam)
{
	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);

	PAINTSTRUCT ps;
	HDC hdcWindow = BeginPaint(m_hwnd, &ps);

	HDC hdc = CreateCompatibleDC(hdcWindow);
	_ASSERTE(hdc != NULL);
	HBITMAP hbm = CreateCompatibleBitmap(hdcWindow, rcClient.right, rcClient.bottom);
	_ASSERTE(hbm != NULL);

	HBITMAP hbmOld = (HBITMAP)SelectObject(hdc, hbm);

	HBRUSH hbrBack = GetSysColorBrush(COLOR_BTNFACE);
	HBRUSH hbrFront = GetSysColorBrush(COLOR_WINDOW);

	HPEN hp = CreatePen(PS_SOLID, DRA::SCALEX(1), RGB(0, 0, 0));
    HPEN hpOld = (HPEN)SelectObject(hdc, hp);

	FillRect(hdc, &rcClient, hbrBack);

	POINT ptsBorder[2] = { {0, DRA::SCALEX(1) - 1}, {rcClient.right, DRA::SCALEX(1) - 1} };
	Polyline(hdc, (LPPOINT)&ptsBorder, sizeof(ptsBorder)/sizeof(POINT));

	SetBkMode(hdc, TRANSPARENT);

	HFONT hfOld = NULL;
	if(m_hf)
	{
		hfOld = (HFONT)SelectObject(hdc, m_hf);
	}

	int dx = 0;

	if(m_bScrollLeftVisible)
	{
		RECT rcScrollLeft = {-1, 0, DRA::SCALEX(TABSTRIP_SCROLL_WIDTH), rcClient.bottom + 1};
		DrawFrameControl(hdc, &rcScrollLeft, DFC_SCROLL, DFCS_SCROLLLEFT | (m_bScrollLeftDown ? DFCS_PUSHED : 0));

		dx += DRA::SCALEX(TABSTRIP_SCROLL_WIDTH);
	}

	
	for(UINT i = 0; i < m_vecItems.Size(); ++i)
	{	
		TabStripItem* pItem = m_vecItems[i];
		_ASSERTE(pItem != NULL);

		if(i >= m_uStart)
		{
			RECT rcText = {dx, 0, dx + pItem->iWidth, rcClient.bottom};

			if(i == m_uSelected)
			{
				FillRect(hdc, &rcText, hbrFront);
			}

			SetTextColor(hdc, pItem->crText);
			DrawText(hdc, pItem->sText.Str(), -1, &rcText, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

			dx += pItem->iWidth;

			POINT ptsDivider[2] = { {dx - 1, 0}, {dx - 1, rcClient.bottom} };
			Polyline(hdc, (LPPOINT)&ptsDivider, sizeof(ptsDivider)/sizeof(POINT));
		}
	}

	if(m_bScrollRightVisible)
	{
		RECT rcScrollRight = {rcClient.right - DRA::SCALEX(TABSTRIP_SCROLL_WIDTH), 0, rcClient.right + 1, rcClient.bottom + 1};
		DrawFrameControl(hdc, &rcScrollRight, DFC_SCROLL, DFCS_SCROLLRIGHT | (m_bScrollRightDown ? DFCS_PUSHED : 0));
	}

	BitBlt(hdcWindow, 0, 0, rcClient.right, rcClient.bottom, hdc, 0, 0, SRCCOPY);

	if(hfOld)
	{
		SelectObject(hdc, hfOld);
	}

	SelectObject(hdc, hpOld);
	SelectObject(hdc, hbmOld);
	SelectObject(hdc, hfOld);

	DeleteObject(hp);
	DeleteObject(hbm);
	DeleteDC(hdc);

	DeleteObject(hbrBack); 
	DeleteObject(hbrFront);

	EndPaint(m_hwnd, &ps);

}

void TabStrip::OnSize(WPARAM wParam, LPARAM lParam)
{
	UpdateScrollButtons();	
}

void TabStrip::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);

	POINT ptHit = {LOWORD(lParam), HIWORD(lParam)};

	int dx = 0;
	if(m_bScrollLeftVisible)
	{
		RECT rcScrollLeft = {-1, 0, DRA::SCALEX(TABSTRIP_SCROLL_WIDTH), rcClient.bottom + 1};
		if(PtInRect(&rcScrollLeft, ptHit))
		{
			SetCapture(m_hwnd);

			m_bScrollLeftDown = true;
			if(m_uStart > 0)
			{
				m_uStart--;
			}

			UpdateScrollButtons();
			Update();
			return;
		}
		dx += DRA::SCALEX(TABSTRIP_SCROLL_WIDTH);
	}

	if(m_bScrollRightVisible)
	{
		RECT rcScrollRight = {rcClient.right + 1 - DRA::SCALEX(TABSTRIP_SCROLL_WIDTH), 0, rcClient.right + 1, rcClient.bottom + 1};
		if(PtInRect(&rcScrollRight, ptHit))
		{
			SetCapture(m_hwnd);

			m_bScrollRightDown = true;

			if(m_uStart < m_vecItems.Size() - 1)
			{
				m_uStart++;
			}

			UpdateScrollButtons();
			Update();
			return;
		}
	}

	for(UINT i = 0; i < m_vecItems.Size(); ++i)
	{	
		TabStripItem* pItem = m_vecItems[i];
		_ASSERTE(pItem != NULL);

		if(i >= m_uStart)
		{
			RECT rcButton = {dx, 0, dx + pItem->iWidth, rcClient.bottom};

			if(PtInRect(&rcButton, ptHit))
			{
				if(m_uSelected != i)
				{
					m_uSelected = i;
					UpdateSelection();
					Update();
				}

				SHRGINFO srgi = {sizeof(SHRGINFO), m_hwnd, {LOWORD(lParam), HIWORD(lParam)}, SHRG_NOTIFYPARENT};
				SHRecognizeGesture(&srgi);

				return;
			}
			dx += pItem->iWidth;
		}
	}
}

void TabStrip::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	ReleaseCapture();
}

void TabStrip::OnCaptureChanged(WPARAM wParam, LPARAM lParam)
{
	if(m_bScrollLeftDown || m_bScrollRightDown)
	{
		m_bScrollLeftDown = false;
		m_bScrollRightDown = false;
		Update();
	}
}

void TabStrip::OnEraseBkgnd(WPARAM wParam, LPARAM lParam)
{

}
/////////////////////////////////////////////////////////////////////////////
// Interface
/////////////////////////////////////////////////////////////////////////////

UINT TabStrip::AddTab(const String& sTitle, HWND hActivate, LPARAM lParam)
{
	_ASSERTE(sTitle.Str() != NULL);

	if(hActivate)
	{
		_ASSERTE(IsWindow(hActivate));
	}

	TabStripItem* pNewItem = new TabStripItem();

	pNewItem->sText = sTitle;
	pNewItem->hActivate = hActivate;
	pNewItem->lParam = lParam;
	pNewItem->iWidth = 0;
	pNewItem->crText = RGB(0, 0, 0);

	pNewItem->iWidth = CalcItemWidth(pNewItem);

	m_vecItems.Append(pNewItem);

	UpdateScrollButtons();
	UpdateSelection();
	Update();

	return (m_vecItems.Size() - 1);
}

bool TabStrip::RemoveTab(UINT uIndex)
{
	if(uIndex < m_vecItems.Size())
	{
		delete m_vecItems[uIndex];
		m_vecItems.Erase(uIndex);

		UpdateSelection();
		UpdateScrollButtons();

		return true;
	}
	return false;
}


TabStrip::TabStripItem* TabStrip::GetTabItem(UINT uIndex)
{
	if(uIndex < m_vecItems.Size())
	{
		return m_vecItems[uIndex];
	}
	return NULL;
}

bool TabStrip::SetTabColor(UINT uIndex, COLORREF color, bool bRedraw)
{
	TabStripItem* pItem = GetTabItem(uIndex);
	if(pItem)
	{
		pItem->crText = color;
		if(bRedraw)
		{
			Update();
		}
		return true;
	}
	return false;
}

bool TabStrip::SetTabText(UINT uIndex, const String& sText, bool bRedraw)
{
	TabStripItem* pItem = GetTabItem(uIndex);
	if(pItem)
	{
		pItem->sText = sText;
		pItem->iWidth = CalcItemWidth(pItem);

		if(bRedraw)
		{
			Update();
		}
		return true;
	}
	return false;
}

bool TabStrip::FindTab(LPARAM lParam, UINT* pResult)
{
	_ASSERTE(pResult != NULL);

	for(UINT i = 0; i < m_vecItems.Size(); ++i)
	{	
		TabStripItem* pItem = m_vecItems[i];
		_ASSERTE(pItem != NULL);

		if(pItem->lParam == lParam)
		{
			*pResult = i;
			return true;
		}
	}
	return false;
}

void TabStrip::Update()
{
	InvalidateRect(m_hwnd, NULL, FALSE);
	UpdateWindow(m_hwnd);
}

bool TabStrip::UpdateSelection()
{
	UINT nItems = m_vecItems.Size();
	if(nItems > 0)
	{
		if(m_uSelected >= nItems)
		{
			m_uSelected = nItems - 1;
		}

		TabStripItem* pItem = GetTabItem(m_uSelected);
		_ASSERTE(pItem != NULL);

		SetWindowPos(pItem->hActivate, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

		if(IsWindow(m_hLastActive) && m_hLastActive != pItem->hActivate)
		{
			ShowWindow(m_hLastActive, SW_HIDE);
			SendMessage(m_hLastActive, WM_ACTIVATE, WA_INACTIVE, 0);
		}

		SendMessage(pItem->hActivate, WM_ACTIVATE, WA_ACTIVE, 0);
		m_hLastActive = pItem->hActivate;
		return true;
	}
	else
	{
		m_uSelected = 0;
		return false;
	}
}

bool TabStrip::GetTabParam(UINT uIndex, LPARAM* pParam)
{
	*pParam = NULL;

	TabStripItem* pItem = GetTabItem(uIndex);
	if(pItem)
	{
		*pParam = pItem->lParam;
	}
	return (pItem != NULL);
}

bool TabStrip::GetCurSel(UINT* pResult)
{
	*pResult = m_uSelected;
	return (m_vecItems.Size() > 0);
}

void TabStrip::SetCurSel(UINT uIndex)
{
	m_uSelected = uIndex;
}

void TabStrip::SetFont(HFONT hf, bool bUpdate)
{
	m_hf = hf;
	if(bUpdate)
	{
		Update();
	}
}

HFONT TabStrip::GetFont()
{
	return m_hf;
}

/////////////////////////////////////////////////////////////////////////////
// Utility Methods
/////////////////////////////////////////////////////////////////////////////

void TabStrip::UpdateScrollButtons()
{
	int dx = 0;
	if(m_uStart > 0)
	{
		m_bScrollLeftVisible = true;
		dx += DRA::SCALEX(TABSTRIP_SCROLL_WIDTH);
	}
	else
	{
		m_bScrollLeftVisible = false;
	}

	for(UINT i = 0; i < m_vecItems.Size(); ++i)
	{	
		TabStripItem* pItem = m_vecItems[i];
		_ASSERTE(pItem != NULL);

		if(i >= m_uStart)
		{
			dx += pItem->iWidth;
		}
	}

	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);

	if(dx > rcClient.right)
	{
		m_bScrollRightVisible = true;
	}
	else
	{
		m_bScrollRightVisible = false;
	}
}

int TabStrip::CalcItemWidth(TabStripItem* pItem)
{
	HDC hdc = GetDC(m_hwnd);
	_ASSERTE(hdc != NULL);
	HFONT hfOld = (HFONT)SelectObject(hdc, m_hf);
	
	RECT rcText = {0, 0, 0, 0};

	DrawText(hdc, pItem->sText.Str(), -1, &rcText, DT_SINGLELINE | DT_CALCRECT);

	SelectObject(hdc, hfOld);
	ReleaseDC(m_hwnd, hdc);	

	return (rcText.right - rcText.left) + (DRA::SCALEX(TABSTRIP_BUTTON_PADDING) * 2);
}

int TabStrip::CalcTabHeight()
{
	HDC hdc = GetDC(m_hwnd);
	_ASSERTE(hdc != NULL);
	HFONT hfOld = (HFONT)SelectObject(hdc, m_hf);
	
	RECT rcText = {0, 0, 0, 0};

	DrawText(hdc, _T("oishii!"), -1, &rcText, DT_SINGLELINE | DT_CALCRECT);

	SelectObject(hdc, hfOld);
	ReleaseDC(m_hwnd, hdc);	

	return (rcText.bottom - rcText.top) + (DRA::SCALEY(TABSTRIP_BUTTON_PADDING));
}

