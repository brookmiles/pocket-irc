#include "PocketIRC.h"
#include "EditMenu.h"

#include "IrcStringFormat.h"

#include "ChooseColor.h"

#include "resource.h"

typedef struct _EDITSUBINFO
{
	WNDPROC wndproc;
	bool bColors;
}EDITSUBINFO;

static LRESULT CALLBACK EditMenuProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EDITSUBINFO *info = (EDITSUBINFO*)GetWindowLong(hwnd, GWL_USERDATA);
	WNDPROC wndproc = info->wndproc;
	switch(msg)
	{
		case WM_LBUTTONDOWN:
		{
			SHRGINFO srgi = {sizeof(SHRGINFO), hwnd, {LOWORD(lParam), HIWORD(lParam)}, SHRG_RETURNCMD};
			if(SHRecognizeGesture(&srgi))
			{
				MapWindowPoints(hwnd, NULL, &srgi.ptDown, 1);
				TrackPopupEditMenu(hwnd, srgi.ptDown.x, srgi.ptDown.y, info->bColors);

				return 0;
			}
		}
		break;
		case WM_DESTROY:
			// Restore old wndproc and free struct
			SetWindowLong(hwnd, GWL_WNDPROC, (LONG)wndproc);
			delete info;
		break;
	}
	return CallWindowProc(wndproc, hwnd, msg, wParam, lParam);
}

void SetEditMenuSubclass(HWND hEdit, bool bColors)
{
	EDITSUBINFO *info = new EDITSUBINFO;
	info->wndproc = (WNDPROC)GetWindowLong(hEdit, GWL_WNDPROC);
	info->bColors = bColors;

	// Store old proc in User Data
	SetWindowLong(hEdit, GWL_USERDATA, (LONG)info);
	SetWindowLong(hEdit, GWL_WNDPROC, (LONG)EditMenuProc);
}

static void DoEditFormat(HWND hEdit, int iCmd)
{
	long lStart = 0;
	long lEnd = 0;
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&lStart, (LPARAM)&lEnd);

	LPCTSTR pszCode = _T("");
	switch(iCmd)
	{
		case ID_EDIT_COLOR_CHOOSE:
			pszCode = _T("\x03");
		break;
		case ID_EDIT_COLOR_BOLD:
			pszCode = _T("\x02");
		break;
		case ID_EDIT_COLOR_INVERSE:
			pszCode = _T("\x16");
		break;
		case ID_EDIT_COLOR_UNDERLINE:
			pszCode = _T("\x1F");
		break;
		case ID_EDIT_COLOR_NORMAL:
			pszCode = _T("\x0F");
		break;
	}

	if(lEnd != lStart && iCmd != ID_EDIT_COLOR_NORMAL)
	{
		SendMessage(hEdit, EM_SETSEL, lEnd, lEnd);
		SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)pszCode);
	}

	SendMessage(hEdit, EM_SETSEL, lStart, lStart);
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)pszCode);

	if(iCmd == ID_EDIT_COLOR_CHOOSE)
	{
		RECT rcEdit;
		GetWindowRect(hEdit, &rcEdit);

		UINT iColor = 0;

		bool bOk = ChooseColourMini(NULL, &iColor, rcEdit.left, rcEdit.top, _T("Text Color"));
		SetFocus(hEdit);
		SendMessage(hEdit, EM_SETSEL, lStart + 1, lStart + 1);

		if(bOk)
		{
			_ASSERTE(iColor < 16);

			TCHAR buf[10];
			wsprintf(buf, _T("%u"), iColor);

			SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)buf);

			bOk = ChooseColourMini(NULL, &iColor, rcEdit.left, rcEdit.top, _T("Back Color"));
			SetFocus(hEdit);
			LONG lPos = lStart + 1 + _tcslen(buf);
			SendMessage(hEdit, EM_SETSEL, lPos, lPos);
	
			if(bOk)
			{
				wsprintf(buf, _T(",%u"), iColor);
				SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)buf);
			}
		}
	}
}

void TrackPopupEditMenu(HWND hEdit, int xPos, int yPos, bool bColors)
{
	HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_EDITMENU));
	_ASSERTE(hMenu != NULL);

	HMENU hEditMenu = GetSubMenu(hMenu, 0);
	_ASSERTE(hEditMenu != NULL);

	if(!bColors)
	{
		EnableMenuItem(hEditMenu, 0, MF_BYPOSITION | MF_GRAYED);
	}

	if(!SendMessage(hEdit, EM_CANUNDO, 0, 0))
	{
		EnableMenuItem(hEditMenu, ID_EDIT_UNDO, MF_BYCOMMAND | MF_GRAYED);
	}

	long lStart = 0;
	long lEnd = 0;
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&lStart, (LPARAM)&lEnd);

	if(lStart == lEnd)
	{
		EnableMenuItem(hEditMenu, ID_EDIT_CUT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hEditMenu, ID_EDIT_COPY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hEditMenu, ID_EDIT_DELETE, MF_BYCOMMAND | MF_GRAYED);
	}

	UINT iCmd = TrackPopupMenuEx(hEditMenu, TPM_RETURNCMD | TPM_BOTTOMALIGN | TPM_LEFTALIGN, xPos, yPos, hEdit, NULL);
	if(iCmd)
	{
		switch(iCmd)
		{
			case ID_EDIT_UNDO:
				SendMessage(hEdit, WM_UNDO, 0, 0);
			break;
			case ID_EDIT_CUT:
				SendMessage(hEdit, WM_CUT, 0, 0);
			break;
			case ID_EDIT_COPY:
				SendMessage(hEdit, WM_COPY, 0, 0);
			break;
			case ID_EDIT_PASTE:
				SendMessage(hEdit, WM_PASTE, 0, 0);
			break;
			case ID_EDIT_DELETE:
				SendMessage(hEdit, WM_CLEAR, 0, 0);
			break;
			case ID_EDIT_CLEAR:
				SendMessage(hEdit, EM_SETSEL, 0, -1);
				SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)_T(""));
			break;
			case ID_EDIT_SELECTALL:
				SendMessage(hEdit, EM_SETSEL, 0, -1);
			break;
			case ID_EDIT_COLOR_CHOOSE:
			case ID_EDIT_COLOR_BOLD:
			case ID_EDIT_COLOR_UNDERLINE:
			case ID_EDIT_COLOR_INVERSE:
			case ID_EDIT_COLOR_NORMAL:
				DoEditFormat(hEdit, iCmd);
			break;
			case ID_EDIT_COLOR_REMOVEALL:
			{
				int len = GetWindowTextLength(hEdit);
				if(len > 0)
				{
					TCHAR* buf = new TCHAR[len + 1];
					GetWindowText(hEdit, buf, len + 1);

					tstring str = StringFormat::StripFormatting(buf);
					delete buf;

					SetWindowText(hEdit, str.c_str());
				}
			}
			break;
		}
	}

	DestroyMenu(hMenu);
}
