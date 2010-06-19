#ifndef _POCKETIRC_H_INCLUDED_
#define _POCKETIRC_H_INCLUDED_

#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <aygshell.h>
#include <winsock.h>
#include <atlconv.h>

#include <list>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include "DeviceResolutionAware.h"

#include "Debug.h"

typedef std::basic_string<TCHAR> tstring;

// Toolbars send this on tap-and-hold
#define TBN_CUSTHELP            (TBN_FIRST-9)

#ifndef CLEARTYPE_QUALITY 
#define CLEARTYPE_QUALITY 5 
#endif
#ifndef CLEARTYPE_COMPAT_QUALITY 
#define CLEARTYPE_COMPAT_QUALITY 6
#endif

#define HANDLEMSGRET(m, h) case m: return (LRESULT)h(wParam, lParam)
#define HANDLEMSG(m, h) case m: h(wParam, lParam); break

#define APP_NAME _T("Pocket IRC")
#define APP_VERSION_STRING _T("1.3")
#define APP_URL _T("http://pocketirc.com/")

#define POCKETIRC_REG_HKEY_ROOT HKEY_CURRENT_USER
#define POCKETIRC_REG_KEY_NAME _T("Software\\Code North\\PocketIRC")
#define POCKETIRC_MAX_REG_SZ_LEN 1024

#define POCKETIRC_DEF_NICK _T("PocketIRC")
#define POCKETIRC_DEF_REALNAME _T("Pocket IRC")
#define POCKETIRC_DEF_IDENT _T("pocketirc")
#define POCKETIRC_DEF_QUIT_MESSAGE _T("Pocket IRC (http://pocketirc.com/)")

#define POCKETIRC_UNREGISTERED_REALNAME _T("Pocket IRC ") APP_VERSION_STRING _T(" UNREGISTERED")
#define POCKETIRC_UNREGISTERED_QUIT_MESSAGE _T("Pocket IRC ") APP_VERSION_STRING _T(" (http://pocketirc.com/) UNREGISTERED")

#define POCKETIRC_VERSION_REPLY APP_NAME _T(" ") APP_VERSION_STRING _T(" ") APP_URL

#define POCKETIRC_DEFAULT_PORT 6667
#define POCKETIRC_MAX_HOST_NAME_LEN 1024
#define POCKETIRC_MAX_NICK_LEN 32
#define POCKETIRC_MAX_REALNAME_LEN 128
#define POCKETIRC_MAX_IRC_LINE_LEN 510
#define POCKETIRC_MAX_IRC_DISPLAY_LEN 640
#define POCKETIRC_FORMAT_CHAR '$'
#define POCKETIRC_WORDWRAP_BIAS 8

#define POCKETIRC_MAX_SAY_LEN			255

#endif//_POCKETIRC_H_INCLUDED_