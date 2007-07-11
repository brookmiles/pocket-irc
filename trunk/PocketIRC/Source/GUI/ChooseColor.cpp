#include "PocketIRC.h"
#include "ChooseColor.h"

#include "ConvertHSL.h"
#include "IrcStringFormat.h"

#include "resource.h"

typedef struct _MINIDATA
{
	UINT *piColorCode;
	LPCTSTR pszTitle;
	int xPos;
	int yPos;
}MINIDATA;

static BOOL CALLBACK ChooseColorMiniProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MINIDATA* pData = (MINIDATA*)GetWindowLong(hwnd, GWL_USERDATA);
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			pData = (MINIDATA*)lParam;

			for(int i = 0; i < 16; i++)
			{
				SetWindowLong(GetDlgItem(hwnd, IDC_CHOOSECOLOR_DEF1 + i), GWL_USERDATA, StringFormat::GetColorCode(i));
			}

			SetWindowText(hwnd, pData->pszTitle);
			
			RECT rcWindow;
			GetWindowRect(hwnd, &rcWindow);
			
			if(pData->xPos < 0)
				pData->xPos = 0;
			else if (pData->xPos + rcWindow.right - rcWindow.left > GetSystemMetrics(SM_CXSCREEN))
				pData->xPos = GetSystemMetrics(SM_CXSCREEN) - (rcWindow.right - rcWindow.left);

			if(pData->yPos < 0)
				pData->yPos = 0;
			else if (pData->yPos > GetSystemMetrics(SM_CYSCREEN))
				pData->yPos = GetSystemMetrics(SM_CYSCREEN);

			SetWindowPos(hwnd, NULL, pData->xPos, pData->yPos - (rcWindow.bottom - rcWindow.top), 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			SipShowIM(SIPF_OFF);
		}
		break;
		case WM_ACTIVATE:
			if(pData && LOWORD(wParam) == WA_INACTIVE)
			{
				EndDialog(hwnd, IDCANCEL);
			}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDCANCEL:
				{
					EndDialog(hwnd, IDCANCEL);
				}
				break;
				default:
					if(HIWORD(wParam) == BN_CLICKED)
					{
						if(pData && LOWORD(wParam) >= IDC_CHOOSECOLOR_DEF1 && LOWORD(wParam) <= IDC_CHOOSECOLOR_DEF16)
						{
							*pData->piColorCode = LOWORD(wParam) - IDC_CHOOSECOLOR_DEF1;

							// Make sure WM_ACTIVATE doesn't call EndDialog again
							SetWindowLong(hwnd, GWL_USERDATA, 0);
							EndDialog(hwnd, IDOK);
						}
					}
			}
		break;
		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *pdi = (DRAWITEMSTRUCT *)lParam;
			SetBkColor(pdi->hDC, GetWindowLong(pdi->hwndItem, GWL_USERDATA));
			ExtTextOut(pdi->hDC, 0, 0, ETO_OPAQUE, &pdi->rcItem, _T(""), 0, NULL);
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}

static BOOL CALLBACK ChooseColorProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CHOOSECOLOR* pcc = (CHOOSECOLOR*)GetWindowLong(hwnd, GWL_USERDATA);
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			pcc = (CHOOSECOLOR*)lParam;

			//// Setup Ok Button and dialog size
			SHINITDLGINFO shidi = {SHIDIM_FLAGS};
			shidi.hDlg = hwnd;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
			SHInitDialog(&shidi);

			//// Create Blank Menu Bar
			SHMENUBARINFO mbi = {sizeof(SHMENUBARINFO)};
			mbi.hwndParent = hwnd;
			mbi.dwFlags = SHCMBF_EMPTYBAR;
			mbi.nToolBarId = 0;
			mbi.hInstRes   = GetModuleHandle(NULL);

			BOOL bCreateMenuOk = SHCreateMenuBar(&mbi);
			_ASSERTE(bCreateMenuOk != FALSE);


			SendDlgItemMessage(hwnd, IDC_CHOOSECOLOR_RED_SPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
			SendDlgItemMessage(hwnd, IDC_CHOOSECOLOR_GREEN_SPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
			SendDlgItemMessage(hwnd, IDC_CHOOSECOLOR_BLUE_SPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));

			COLORREF cr = pcc->rgbResult;
			SetDlgItemInt(hwnd, IDC_CHOOSECOLOR_RED, GetRValue(cr), FALSE);
			SetDlgItemInt(hwnd, IDC_CHOOSECOLOR_GREEN, GetGValue(cr), FALSE);
			SetDlgItemInt(hwnd, IDC_CHOOSECOLOR_BLUE, GetBValue(cr), FALSE);

			SetWindowLong(GetDlgItem(hwnd, IDC_CHOOSECOLOR_NEW), GWL_USERDATA, pcc->rgbResult);
			SetWindowLong(GetDlgItem(hwnd, IDC_CHOOSECOLOR_OLD), GWL_USERDATA, pcc->rgbResult);

			for(int i = 0; i < 16; i++)
			{
				SetWindowLong(GetDlgItem(hwnd, IDC_CHOOSECOLOR_DEF1 + i), GWL_USERDATA, StringFormat::GetColorCode(i));
			}
		}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					int iRed = GetDlgItemInt(hwnd, IDC_CHOOSECOLOR_RED, NULL, FALSE);
					int iGreen = GetDlgItemInt(hwnd, IDC_CHOOSECOLOR_GREEN, NULL, FALSE);
					int iBlue = GetDlgItemInt(hwnd, IDC_CHOOSECOLOR_BLUE, NULL, FALSE);

					pcc->rgbResult = RGB(iRed, iGreen, iBlue);

					EndDialog(hwnd, IDOK);
				}
				break;
				case IDC_CHOOSECOLOR_RED:
				case IDC_CHOOSECOLOR_GREEN:
				case IDC_CHOOSECOLOR_BLUE:
				{
					if(pcc && HIWORD(wParam) == EN_CHANGE)
					{
						int iRed = GetDlgItemInt(hwnd, IDC_CHOOSECOLOR_RED, NULL, FALSE);
						int iGreen = GetDlgItemInt(hwnd, IDC_CHOOSECOLOR_GREEN, NULL, FALSE);
						int iBlue = GetDlgItemInt(hwnd, IDC_CHOOSECOLOR_BLUE, NULL, FALSE);

						pcc->rgbResult = RGB(iRed, iGreen, iBlue);
						SetWindowLong(GetDlgItem(hwnd, IDC_CHOOSECOLOR_NEW), GWL_USERDATA, pcc->rgbResult);

						InvalidateRect(GetDlgItem(hwnd, IDC_CHOOSECOLOR_NEW), NULL, FALSE);
						UpdateWindow(GetDlgItem(hwnd, IDC_CHOOSECOLOR_NEW));
					}
				}
				break;
				default:
					if(HIWORD(wParam) == BN_CLICKED)
					{
						if(pcc)
						{
							LONG cr = GetWindowLong((HWND)lParam, GWL_USERDATA);

							SetDlgItemInt(hwnd, IDC_CHOOSECOLOR_RED, GetRValue(cr), FALSE);
							SetDlgItemInt(hwnd, IDC_CHOOSECOLOR_GREEN, GetGValue(cr), FALSE);
							SetDlgItemInt(hwnd, IDC_CHOOSECOLOR_BLUE, GetBValue(cr), FALSE);
						}
					}
			}
		break;
		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *pdi = (DRAWITEMSTRUCT *)lParam;
/*
			if(wParam == IDC_CHOOSECOLOR_PICKER)
			{
				for(int x = 0; x < pdi->rcItem.right; ++x)
				{
					for(int y = 0; y < pdi->rcItem.bottom; ++y)
					{
						COLORREF cr = HSLtoRGB(double(x) / double(pdi->rcItem.right), double(pdi->rcItem.bottom - y) / double(pdi->rcItem.bottom), 0.5);
						SetPixel(pdi->hDC, x, y, cr);
					}
				}
			}
*/
			SetBkColor(pdi->hDC, GetWindowLong(pdi->hwndItem, GWL_USERDATA));
			ExtTextOut(pdi->hDC, 0, 0, ETO_OPAQUE, &pdi->rcItem, _T(""), 0, NULL);
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}

bool ChooseColour(CHOOSECOLOR* pcc)
{
	LONG lRet = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CHOOSECOLOR), 
		pcc->hwndOwner, ChooseColorProc, (LONG)pcc);
	return (lRet == IDOK);
}

bool ChooseColourMini(HWND hParent, UINT* piColorCode, int xPos, int yPos, LPCTSTR pszTitle)
{
	MINIDATA md;
	md.piColorCode = piColorCode;
	md.pszTitle = pszTitle;
	md.xPos = xPos;
	md.yPos = yPos;

	LONG lRet = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CHOOSECOLOR_MINI), 
		hParent, ChooseColorMiniProc, (LONG)&md);
	bool bOk = (lRet == IDOK);
	return bOk;
}
