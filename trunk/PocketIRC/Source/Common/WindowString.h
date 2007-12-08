#ifndef _WINDOWSTRING_H_INCLUDED_
#define _WINDOWSTRING_H_INCLUDED_

bool GetWindowString(HWND hwnd, tstring& str);
bool GetDlgItemString(HWND hwnd, UINT id, tstring& str);

#endif//_WINDOWSTRING_H_INCLUDED_
