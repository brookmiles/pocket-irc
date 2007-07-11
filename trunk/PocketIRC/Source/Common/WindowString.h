#ifndef _WINDOWSTRING_H_INCLUDED_
#define _WINDOWSTRING_H_INCLUDED_

bool GetWindowString(HWND hwnd, String& str);
bool GetDlgItemString(HWND hwnd, UINT id, String& str);

#endif//_WINDOWSTRING_H_INCLUDED_
