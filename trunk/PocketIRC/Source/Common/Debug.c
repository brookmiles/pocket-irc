#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Debug.h"

#define MESSAGEBOXFMT_BUF_SIZE	256

#define DEBUG_REPORT_BUF_SIZE	128
#define DEBUG_ABORT_CODE		0xBADD061E

#define DEBUG_TRACE_BUF_SIZE 1024
#define DEBUG_TRACE_FMT_SIZE 128


UINT MessageBoxFmt(HWND hwnd, LPCTSTR pszTextFmt, LPCTSTR pszTitle, UINT uType, ...)
{
	TCHAR buf[MESSAGEBOXFMT_BUF_SIZE];
	va_list args;
	va_start(args, uType);
	_vsntprintf(buf, sizeof(buf), pszTextFmt, args);
	va_end(args);
	return MessageBox(hwnd, buf, pszTitle, uType);
}

#if defined(DEBUG)
/////////////////////////////////////////////////////////////////////////////
//	_DebugReport is used by the ASSERTE macro.  Since the preprocessor
//	constants __FILE__ and __LINE__ are always ANSI, it takes LPCSTR
//	instead of LPCTSTR, and then converts to wide strings.
//

int _DebugReport(LPCSTR pszFile, int iLine, LPCSTR pszExpr, UINT gle, BOOL bConsoleOnly)
{
	char buf[DEBUG_REPORT_BUF_SIZE];
	wchar_t wbuf[DEBUG_REPORT_BUF_SIZE];
	wchar_t wbufTitle[DEBUG_REPORT_BUF_SIZE];
	UINT ret;
	
	_snprintf(buf, DEBUG_REPORT_BUF_SIZE, "Assertion Failed! (0x%08X)", gle);
	mbstowcs(wbufTitle, buf, DEBUG_REPORT_BUF_SIZE/2);

	_snprintf(buf, DEBUG_REPORT_BUF_SIZE, "%s:%d\r\n%s\r\nContinue?", 
		strrchr(pszFile, '\\') + 1, iLine, pszExpr);
	mbstowcs(wbuf, buf, DEBUG_REPORT_BUF_SIZE);

	OutputDebugString(wbufTitle);
	OutputDebugString(_T("\r\n"));
	OutputDebugString(wbuf);
	OutputDebugString(_T("\r\n"));

	if(!bConsoleOnly)
	{
		ret = MessageBox(NULL, wbuf, wbufTitle, MB_ICONERROR | MB_ABORTRETRYIGNORE);
		if(ret == IDABORT)
			ExitThread(DEBUG_ABORT_CODE);
		return (ret == IDRETRY);
	}
	else
	{
		return TRUE;
	}
}

void _TRACE(LPCSTR fmt, ...)
{
	int len;
	TCHAR buf[DEBUG_TRACE_BUF_SIZE + 3] = _T("");
	va_list va;

#ifndef UNICODE
	char* tfmt = fmt;
#else
	wchar_t tfmt[DEBUG_TRACE_FMT_SIZE];
	mbstowcs(tfmt, fmt, DEBUG_TRACE_FMT_SIZE);
#endif

	va_start(va, fmt);
	len = _vsntprintf(buf, DEBUG_TRACE_BUF_SIZE, tfmt, va);
	va_end(va);

	if(len >= -1)
	{
		if(len == -1)
		{
			len = DEBUG_TRACE_BUF_SIZE;
		}
		lstrcpy(&buf[len], _T("\r\n"));
	}
	else
	{
		lstrcpy(buf, _T("DEBUG FORMATTING ERROR\r\n"));
	}

	OutputDebugString(buf);
}

#endif//defined(DEBUG)