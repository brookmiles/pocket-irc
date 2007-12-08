#include "PocketIRC.h"
#include "WindowString.h"

bool GetWindowString(HWND hwnd, tstring& str)
{
	int len = GetWindowTextLength(hwnd);
	if(len > 0)
	{
		TCHAR* buf = new TCHAR[len + 1];

		GetWindowText(hwnd, buf, len + 1);

		str = buf;
		delete[] buf;

		return true;
	}
	return false;
}

bool GetDlgItemString(HWND hwnd, UINT id, tstring& str)
{
	return GetWindowString(GetDlgItem(hwnd, id), str);
}
