#include "PocketIRC.h"
#include "Dlg.h"

/////////////////////////////////////////////////////////////////////////////
//	Stub Window Procedure
/////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK Dlg::DlgProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Dlg* pWnd = (Dlg*)GetWindowLong(hwnd, GWL_USERDATA);
	if(pWnd)
	{
		return pWnd->DlgProc(msg, wParam, lParam);
	}
	else
	{
		if(msg == WM_INITDIALOG)
		{
			pWnd = (Dlg*)lParam;
			_ASSERTE(pWnd != NULL);

			pWnd->m_hwnd = hwnd;
			SetWindowLong(hwnd, GWL_USERDATA, (LONG)pWnd);

			return pWnd->DlgProc(msg, wParam, lParam);
		}
		else
		{
			return FALSE;
		}
	}	
}

BOOL Dlg::DefProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			//// Create Blank Menu Bar
			SHINITDLGINFO shidi = {SHIDIM_FLAGS};
			shidi.hDlg = m_hwnd;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_SIPDOWN;
			SHInitDialog(&shidi);

			SHMENUBARINFO mbi = {sizeof(SHMENUBARINFO)};
			mbi.hwndParent = m_hwnd;
			mbi.dwFlags = SHCMBF_EMPTYBAR;
			mbi.nToolBarId = 0;
			mbi.hInstRes   = GetModuleHandle(NULL);

			BOOL bCreateMenuOk = SHCreateMenuBar(&mbi);
			_ASSERTE(bCreateMenuOk != FALSE);
		}
		break;
		case WM_SETTINGCHANGE:
			if(wParam == SPI_SETSIPINFO)
			{
				SHHandleWMSettingChange(m_hwnd, wParam, lParam, &m_shackInfo);
			}
		break;
		case WM_ACTIVATE:
			SHHandleWMActivate(m_hwnd, wParam, lParam, &m_shackInfo, FALSE);
		break;
		default:
			return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

Dlg::Dlg(HINSTANCE hInst, UINT uID)
{
	_ASSERTE(hInst != NULL);
	_ASSERTE(uID != 0);

	m_hInst = hInst;
	m_uID = uID;
	m_hwnd = NULL;

	ZeroMemory(&m_shackInfo, sizeof(m_shackInfo));
	m_shackInfo.cbSize = sizeof(m_shackInfo);
}

Dlg::~Dlg()
{

}

/////////////////////////////////////////////////////////////////////////////
//	Creation
/////////////////////////////////////////////////////////////////////////////

int Dlg::DoModal(HWND hParent)
{
	return DialogBoxParam(m_hInst, MAKEINTRESOURCE(m_uID), hParent,	Dlg::DlgProcStub, (LPARAM)this);
}
