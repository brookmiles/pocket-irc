#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>

#include "TabCtrlUtil.h"

void TabCtrl_FitToParent(HWND hTab)
{
	RECT rcClient;
	RECT rcTab;

	GetClientRect(GetParent(hTab), &rcClient);

	SetWindowPos(hTab, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);

	GetClientRect(hTab, &rcTab);
	TabCtrl_AdjustRect(hTab, FALSE, &rcTab);

	rcTab.top = 0;
	rcTab.left = 0;
	rcTab.right = rcClient.right;
	TabCtrl_AdjustRect(hTab, TRUE, &rcTab);

	SetWindowPos(hTab, NULL, rcTab.left, rcTab.top, rcClient.right - rcTab.left, rcClient.bottom - rcTab.top, SWP_NOZORDER);
}

