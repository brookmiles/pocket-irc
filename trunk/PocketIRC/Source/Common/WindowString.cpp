#include "PocketIRC.h"
#include "StringT.h"
#include "WindowString.h"

bool GetWindowString(HWND hwnd, String& str)
{
	int len = GetWindowTextLength(hwnd);
	if(len > 0)
	{
		str.Reserve(len + 1);
		GetWindowText(hwnd, str.Str(), str.Capacity());

		return true;
	}
	return false;
}

bool GetDlgItemString(HWND hwnd, UINT id, String& str)
{
	return GetWindowString(GetDlgItem(hwnd, id), str);
}
