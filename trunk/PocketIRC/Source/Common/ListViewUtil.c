#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>

#include "ListViewUtil.h"

void 
ListView_SetItemInt(HWND hList, int iItem, int iSubItem, int i)
{
    TCHAR buf[50];
    _stprintf(buf, _T("%d"), i);
    ListView_SetItemText(hList, iItem, iSubItem, buf);
}

void ListView_SetItemParam(HWND hList, int iItem, LPARAM lParam)
{
    LVITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.lParam = lParam;
    ListView_SetItem(hList, &lvi);
}

LPARAM
ListView_GetItemParam(HWND hList, int iItem)
{
    LVITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.lParam = 0;
    ListView_GetItem(hList, &lvi);
    return lvi.lParam;
}

long 
ListView_EnumItems(HWND hList, ListViewEnumCB cb, long l)
{
    TCHAR buf[256];
    LVITEM lvi;
    lvi.mask = LVIF_PARAM | LVIF_IMAGE | LVIF_STATE | LVIF_INDENT | LVIF_TEXT;
    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = buf;
    lvi.cchTextMax = sizeof(buf)/sizeof(TCHAR);
    while(ListView_GetItem(hList, &lvi)){
        long ret = cb(&lvi, l);
        if(ret)
            return ret;
        lvi.iItem++;
    }
    return 0;
}

long
ListView_FindItemByParamCB(LPLVITEM plvi, long lParam)
{
    if(plvi->lParam == lParam)
        return plvi->iItem + 1;
    else
        return 0;
}

int 
ListView_FindItemByParam(HWND hList, LPARAM lParam)
{
    // here we get a 1 based index back, with 0 indicating
    // failure.  this is because if it was 0 based the 0 wouldn't
    // break the enumeration, so we use 1 instead see? no? too bad.
    int index = ListView_EnumItems(hList, ListView_FindItemByParamCB, lParam);
    return index - 1;
}
