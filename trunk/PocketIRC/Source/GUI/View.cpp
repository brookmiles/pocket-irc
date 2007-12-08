#include "PocketIRC.h"
#include "View.h"

#include "Config\Options.h"

#include "IrcString.h"

#define POCKETIRC_BACKBUFFER_SIZE 100

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

View::View()
{
	_TRACE("View(0x%08X)::View()", this);

	m_iStart = 0;
	m_sel.valid = false;

	ZeroMemory(&m_rcOldClient, sizeof(RECT));
}

View::~View()
{
	_TRACE("View(0x%08X)::~View()", this);

	Clear();

	//DeleteObject(m_hf);  don't do this, we share font handle
}

/////////////////////////////////////////////////////////////////////////////
//	Window Creation
/////////////////////////////////////////////////////////////////////////////

HRESULT View::Create(HWND hParent, UINT iID, int x, int y, int w, int h)
{
	HRESULT hr = Window::Create(hParent, _T("i r teh view"), WS_CHILD | WS_VISIBLE, iID, x, y, w, h);
	_ASSERTE(SUCCEEDED(hr));

	return hr;
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

void View::AddLine(const tstring& sText)
{
	ViewMsg* pMsg = new ViewMsg;
	_ASSERTE(pMsg != NULL);

	pMsg->msg = sText;
	pMsg->cy = 0;

	m_vecMsgs.push_back(pMsg);
	UINT nMsgs = m_vecMsgs.size();
	
	if(nMsgs > POCKETIRC_BACKBUFFER_SIZE)
	{
		for(UINT i = 0; i < 10; ++i)
		{
			delete m_vecMsgs[i];
			m_vecMsgs[i] = NULL;
		}

		m_vecMsgs.erase(m_vecMsgs.begin(), m_vecMsgs.begin() + 10);
	}

	if(m_iStart)
	{
		m_iStart++;
	}

	if(m_iStart >= m_vecMsgs.size())
	{
		m_iStart = m_vecMsgs.size() - 1;
	}

	UpdateScrollBar();
	Update();
}

void View::Update(bool bUpdateBounds)
{
	if(bUpdateBounds)
	{
		ClearBounds();
		UpdateScrollBar();
	}

	HDC hdc = GetDC(m_hwnd);

	DrawUpdate(hdc);
	ValidateRect(m_hwnd, NULL);

	ReleaseDC(m_hwnd, hdc);
}


void View::ScrollIndex(UINT iScroll)
{
	if(iScroll < m_vecMsgs.size())
	{
		m_iStart = iScroll;

		SCROLLINFO si = {sizeof(si), SIF_POS};
		si.nPos = m_vecMsgs.size() - 1 - m_iStart;

		SetScrollInfo(m_hwnd, SB_VERT, &si, TRUE);
		
		Update();
	}
}

void View::Clear()
{
	for(UINT i = 0; i < m_vecMsgs.size(); ++i)
	{
		delete m_vecMsgs[i];
	}
	m_vecMsgs.clear();
}

tstring View::GetWordAtPoint(POINT pt)
{
	UINT iMsg;
	UINT iChar;
	if(HitTestMsg((USHORT)pt.x, (USHORT)pt.y, &iMsg, &iChar))
	{
		return GetWordAtChar(iMsg, iChar);
	}
	return _T("");
}

/////////////////////////////////////////////////////////////////////////////
//	Internal Methods
/////////////////////////////////////////////////////////////////////////////

void View::UpdateScrollBar()
{
	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);

	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nPage = 10;
	si.nMin = 0;
	si.nMax = m_vecMsgs.size() - 2 + si.nPage;
	si.nPos = m_vecMsgs.size() - 1 - m_iStart;
	SetScrollInfo(m_hwnd, SB_VERT, &si, TRUE);
}

void View::DrawUpdate(HDC hdc)
{
	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);

	HBITMAP hbm = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
	_ASSERTC(hbm != NULL);

	HDC hdcMem = CreateCompatibleDC(hdc);
	_ASSERTC(hdcMem != NULL);

	HFONT hfOld = (HFONT)SelectObject(hdcMem, g_Options.GetDisplayFont());
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);

	SetBkColor(hdcMem, g_Options.GetBackColor());
	ExtTextOut(hdcMem, 0, 0, ETO_OPAQUE, &rcClient, _T(""), 0, NULL);

	HBITMAP hbmBack = g_Options.GetDisplayBackground();
	if(hbmBack)
	{
		BITMAP bm;
		GetObject(hbmBack, sizeof(BITMAP), &bm);

		HDC hdcBack = CreateCompatibleDC(hdc);
		HBITMAP hbmBackOld = (HBITMAP)SelectObject(hdcBack, hbmBack);

		if(g_Options.GetBackAlignTop())
		{
			BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcBack, 0, 0, SRCCOPY);
		}
		else
		{
			BitBlt(hdcMem, 0, rcClient.bottom - bm.bmHeight, bm.bmWidth, bm.bmHeight, hdcBack, 0, 0, SRCCOPY);
		}

		SelectObject(hdcBack, hbmBackOld);
		DeleteDC(hdcBack);
	}

	Draw(hdcMem, rcClient);
	BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hbmOld);
	SelectObject(hdcMem, hfOld);

	DeleteDC(hdcMem);
	DeleteObject(hbm);
}

UINT View::Draw(HDC hdc, const RECT& rect)
{
	//_TRACE("View(0x%08X)::Draw()", this);

	_ASSERTC(hdc != NULL);

	UINT nUsed = 0;

	StringFormat fmt;
	fmt.bg = -1;
	fmt.fg = g_Options.GetTextColor();
	fmt.bold = false;
	fmt.underline = false;
	fmt.inverse = false;

	m_Formatter.SetDefaultFmt(fmt);

	if(m_vecMsgs.size() > 0)
	{
		for(UINT i = m_vecMsgs.size() - m_iStart; i > 0; --i)
		{
			ViewMsg *msg = m_vecMsgs[i - 1];
			_ASSERTC(msg != NULL);

			if(msg->cy == 0)
				UpdateMsgBounds(*msg, hdc, rect.right - rect.left);

			UINT nMsgHeight = DrawMsg(*msg, hdc, rect.right - rect.left, rect.bottom - nUsed - msg->cy);

			if(m_sel.valid)
			{
				DrawSelection(*msg, hdc, rect.right - rect.left, rect.bottom - nUsed - msg->cy, i - 1);
			}

			nUsed += nMsgHeight;
			if(nUsed >= UINT(rect.bottom - rect.top))
				break;
		}
	}
	return nUsed;
}

UINT View::DrawMsg(ViewMsg& msg, HDC hdc, UINT cx, UINT dy)
{
	RECT rcDraw = {0, dy, cx, 0};

	if(g_Options.GetStripAll())
	{
		m_Formatter.FormatOut(hdc, StringFormat::StripFormatting(msg.msg), rcDraw, Formatter::FMT_WRAP);
	}
	else
	{
		m_Formatter.FormatOut(hdc, msg.msg, rcDraw, Formatter::FMT_WRAP);
	}

	return rcDraw.bottom - rcDraw.top;
}

void View::DrawSelection(ViewMsg& msg, HDC hdc, UINT cx, UINT dy, UINT iMsg)
{
	RECT rcDraw = {0, dy, cx, 0};

	UINT iMsgStart = m_sel.iMsgStart;
	UINT iMsgEnd = m_sel.iMsgEnd;
	UINT iSelStart = m_sel.iCharStart;
	UINT iSelEnd = m_sel.iCharEnd;

	// Swap start and end if they are in the wrong order
	if(iMsgStart > iMsgEnd)
	{
		iMsgStart = m_sel.iMsgEnd;
		iMsgEnd = m_sel.iMsgStart;
		iSelStart = m_sel.iCharEnd;
		iSelEnd = m_sel.iCharStart;
	}
	else if(iMsgStart == iMsgEnd && iSelStart > iSelEnd)
	{
		iSelStart = m_sel.iCharEnd;
		iSelEnd = m_sel.iCharStart;
	}

	if(iMsg >= iMsgStart && iMsg <= iMsgEnd)
	{

		if(iMsg > iMsgStart) 
			iSelStart = 0;

		if(iMsg < iMsgEnd) 
			iSelEnd = -1;

		m_Formatter.DrawSelection(hdc, msg.msg, rcDraw, Formatter::FMT_WRAP, iSelStart, iSelEnd);
	}
}

void View::UpdateMsgBounds(ViewMsg& msg, HDC hdc, UINT cx)
{
	_ASSERTC(hdc != NULL);
	_ASSERTC(cx >= 0);

	RECT rcDraw = {0, 0, cx, 0};

	m_Formatter.FormatOut(hdc, msg.msg, rcDraw, Formatter::FMT_WRAP | Formatter::FMT_CALC);

	msg.cy = rcDraw.bottom;
}

void View::ClearBounds()
{
	for(UINT i = 0; i < m_vecMsgs.size(); ++i)
	{
		m_vecMsgs[i]->cy = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Selection Handling
/////////////////////////////////////////////////////////////////////////////

bool View::StartSel(WORD x, WORD y)
{
	UINT iMsgStart;
	UINT iCharStart;
	if(HitTestMsg(x, y, &iMsgStart, &iCharStart))
	{
		_TRACE("View(0x%08X)::StartSel() valid = true", this);
		m_sel.valid = true;

		m_sel.iMsgStart = iMsgStart;
		m_sel.iCharStart = iCharStart;
		m_sel.iMsgEnd = iMsgStart;
		m_sel.iCharEnd = iCharStart;
	}
	else
	{
		_TRACE("View(0x%08X)::StartSel() valid = false", this);
		m_sel.valid = false;
	}
	return m_sel.valid;
}

bool View::TrackSel(WORD x, WORD y)
{
	if(m_sel.valid)
	{
		UINT iMsgTrack;
		UINT iCharTrack;
		if(HitTestMsg(x, y, &iMsgTrack, &iCharTrack))
		{
			m_sel.iMsgEnd = iMsgTrack;
			m_sel.iCharEnd = iCharTrack;
		}
		return true;
	}
	return false;
}

bool View::EndSel(bool bCopy)
{
	if(m_sel.valid)
	{
		if(bCopy)
		{
			if(m_sel.iMsgStart == m_sel.iMsgEnd && 
				m_sel.iCharStart == m_sel.iCharEnd)
			{
				NMHDR nmhdr = {m_hwnd, GetDlgCtrlID(m_hwnd), NM_CLICK};
				SendMessage(GetParent(m_hwnd), WM_NOTIFY, (WPARAM)GetDlgCtrlID(m_hwnd), (LPARAM)&nmhdr);
			}
			else
			{
				CopySelection();
			}
		}
		_TRACE("View(0x%08X)::EndSel() valid = false", this);
		m_sel.valid = false;
		return true;
	}
	return false;
}

bool View::HitTestMsg(WORD x, WORD y, UINT* piMsgStart, UINT* piCharStart)
{
	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);

	POINT pt = {x, y};

	HDC hdc = GetDC(m_hwnd);
	_ASSERTC(hdc != NULL);
	HFONT hfOld = (HFONT)SelectObject(hdc, g_Options.GetDisplayFont());

	bool bSuccess = false;
	long cyUsed = 0;
	for(UINT i = m_vecMsgs.size() - m_iStart; i > 0; --i)
	{
		ViewMsg *msg = m_vecMsgs[i - 1];
		_ASSERTC(msg != NULL);

		if(msg->cy == 0)
			UpdateMsgBounds(*msg, hdc, rcClient.right - rcClient.left);

		RECT rcMsg = {rcClient.left, rcClient.bottom - cyUsed - msg->cy, rcClient.right, rcClient.bottom - cyUsed};
		if(PtInRect(&rcMsg, pt))
		{
			*piMsgStart = i - 1;
			//*piCharStart = FormatText(hdc, &rcMsg, msg, &pt, NULL, FT_HITTEST);
			*piCharStart = m_Formatter.HitTest(hdc, msg->msg, rcMsg, Formatter::FMT_WRAP, pt);
			bSuccess = true;
			break;
		}

		cyUsed += msg->cy;
		if(cyUsed >= rcClient.bottom - rcClient.top)
			break;
	}

	SelectObject(hdc, hfOld);
	ReleaseDC(m_hwnd, hdc);

	return bSuccess;
}

void View::CopySelection()
{
	UINT iMsgStart = m_sel.iMsgStart;
	UINT iMsgEnd = m_sel.iMsgEnd;
	UINT iSelStart = m_sel.iCharStart;
	UINT iSelEnd = m_sel.iCharEnd;

	// Swap start and end if they are in the wrong order
	if(iMsgStart > iMsgEnd)
	{
		iMsgStart = m_sel.iMsgEnd;
		iMsgEnd = m_sel.iMsgStart;
		iSelStart = m_sel.iCharEnd;
		iSelEnd = m_sel.iCharStart;
	}
	else if(iMsgStart == iMsgEnd && iSelStart > iSelEnd)
	{
		iSelStart = m_sel.iCharEnd;
		iSelEnd = m_sel.iCharStart;
	}

	tstring str;
	str.reserve(1024*8);

	for(UINT i = iMsgStart; i <= iMsgEnd; ++i)
	{
		ViewMsg *msg = m_vecMsgs[i];
		_ASSERTC(msg != NULL);

		UINT iMsgCharStart = (i > iMsgStart) ? 0 : iSelStart;
		UINT nMsgCharMax = (i < iMsgEnd) ? -1 : iSelEnd - iMsgCharStart;

		str += StringFormat::StripFormatting(msg->msg).substr(iMsgCharStart, nMsgCharMax);
		if(i < iMsgEnd)
		{
			str += _T("\r\n");
		}
	}

	if(OpenClipboard(m_hwnd))
	{
		if(EmptyClipboard())
		{
			LPTSTR hMem = (LPTSTR)GlobalAlloc(LPTR, (str.size() + 1) * sizeof(TCHAR));
			_tcscpy(hMem, str.c_str());

			HANDLE hSetOk = SetClipboardData(CF_UNICODETEXT, (HANDLE)hMem);
			_ASSERTC(hSetOk != NULL);
		}
		CloseClipboard();
	}
}

tstring View::GetWordAtChar(UINT iMsg, UINT iChar)
{
	if(iMsg < m_vecMsgs.size())
	{
		ViewMsg* pMsg = m_vecMsgs[iMsg];
		_ASSERTE(pMsg != NULL);

		tstring sMsg = StringFormat::StripFormatting(pMsg->msg);
		if(iChar < sMsg.size())
		{
			UINT iWordStart = iChar;
			while(iWordStart > 0 && !_istspace(sMsg[iWordStart - 1]))
				iWordStart--;

			UINT iWordEnd = iChar;
			while(iWordEnd < sMsg.size() && !_istspace(sMsg[iWordEnd]))
				iWordEnd++;

			return sMsg.substr(iWordStart, iWordEnd - iWordStart);
		}
	}
	return _T("");
}

tstring View::GetSelectedWord()
{
	return GetWordAtChar(m_sel.iMsgStart, m_sel.iCharStart);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Procedure
/////////////////////////////////////////////////////////////////////////////

LRESULT View::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLEMSGRET(WM_CREATE, OnCreate);
		HANDLEMSG(WM_DESTROY, OnDestroy);
		HANDLEMSG(WM_SIZE, OnSize);
		HANDLEMSG(WM_PAINT, OnPaint);
		HANDLEMSGRET(WM_ERASEBKGND, OnEraseBkgnd);
		HANDLEMSG(WM_VSCROLL, OnVScroll);
		HANDLEMSG(WM_LBUTTONDOWN, OnLButtonDown);
		HANDLEMSG(WM_LBUTTONUP, OnLButtonUp);
		HANDLEMSG(WM_MOUSEMOVE, OnMouseMove);
		HANDLEMSG(WM_CAPTURECHANGED, OnCaptureChanged);
		default:
			return DefWindowProc(m_hwnd, msg, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//	Message Handling
/////////////////////////////////////////////////////////////////////////////

LRESULT View::OnCreate(WPARAM wParam, LPARAM lParam)
{
	_TRACE("View(0x%08X)::OnCreate()", this);
	SetWindowLong(m_hwnd, GWL_EXSTYLE, (GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_NOACTIVATE));
	return 0;
}

void View::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	_TRACE("View(0x%08X)::OnDestroy()", this);

	Clear();

	Window::OnDestroy(wParam, lParam);
}

void View::OnSize(WPARAM wParam, LPARAM lParam)
{
	_TRACE("View(0x%08X)::OnSize()", this);

	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);
	if(rcClient.right != m_rcOldClient.right || rcClient.bottom != m_rcOldClient.bottom)
	{
		Update(true);
	}
	CopyMemory(&m_rcOldClient, &rcClient, sizeof(RECT));
}

void View::OnPaint(WPARAM wParam, LPARAM lParam)
{
	_TRACE("View(0x%08X)::OnPaint()", this);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hwnd, &ps);

	DrawUpdate(hdc);

	EndPaint(m_hwnd, &ps);

}

LRESULT View::OnEraseBkgnd(WPARAM wParam, LPARAM lParam)
{
	_TRACE("View(0x%08X)::OnEraseBkgnd()", this);
	return TRUE;
}

void View::OnVScroll(WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO si = {sizeof(si), SIF_ALL};
	GetScrollInfo(m_hwnd, SB_VERT, &si);

	switch(LOWORD(wParam))
	{
		case SB_THUMBPOSITION:
			si.nPos = si.nTrackPos;
		break;
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
		break;
		case SB_LINEDOWN:
			si.nPos++;
		break;
		case SB_LINEUP:
			si.nPos--;
		break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
		break;
		case SB_PAGEUP:
			si.nPos -= si.nPage;
		break;
		case SB_BOTTOM:
			si.nPos = si.nMax;
		break;
		case SB_TOP:
			si.nPos = si.nMin;
		break;
	}

	si.fMask = SIF_POS;

	SetScrollInfo(m_hwnd, SB_VERT, &si, TRUE);
	GetScrollInfo(m_hwnd, SB_VERT, &si);

	UINT iNewStart = m_vecMsgs.size() - 1 - si.nPos;
	_ASSERTC(iNewStart < m_vecMsgs.size());

	if(iNewStart != m_iStart)
	{
		m_iStart = iNewStart;
		Update();
	}
}

void View::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	_TRACE("View(0x%08X)::OnLButtonDown()", this);
	SHRGINFO srgi = {sizeof(SHRGINFO), m_hwnd, {LOWORD(lParam), HIWORD(lParam)}, SHRG_NOTIFYPARENT};
	if(SHRecognizeGesture(&srgi) == 0)
	{
		SetCapture(m_hwnd);
		StartSel(LOWORD(lParam), HIWORD(lParam));
	}
	else
	{
		EndSel(false);
	}
}

void View::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	_TRACE("View(0x%08X)::OnLButtonUp()", this);
	if(EndSel(true))
	{
		Update();
	}

	if(m_hwnd == GetCapture())
	{
		ReleaseCapture();
	}
}

void View::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	if(TrackSel(LOWORD(lParam), HIWORD(lParam)))
	{
		Update();
	}
}

void View::OnCaptureChanged(WPARAM wParam, LPARAM lParam)
{
	if((HWND)lParam != m_hwnd)
	{
		_TRACE("View(0x%08X)::OnCaptureChanged()", this);
		if(EndSel(false))
		{
			Update();
		}
	}
}
