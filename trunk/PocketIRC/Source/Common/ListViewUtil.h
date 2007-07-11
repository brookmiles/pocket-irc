#ifndef _LISTVIEW_UTIL_H_INCLUDED_
#define _LISTVIEW_UTIL_H_INCLUDED_
#ifdef __cplusplus
extern "C" {
#endif

typedef long (*ListViewEnumCB)(LPLVITEM, long);

void ListView_SetItemInt(HWND hList, int iItem, int iSubItem, int i);
void ListView_SetItemParam(HWND hList, int iItem, LPARAM lParam);
LPARAM ListView_GetItemParam(HWND hList, int iItem);

int ListView_FindItemByParam(HWND hList, LPARAM lParam);

long ListView_EnumItems(HWND, ListViewEnumCB, long);

#ifdef __cplusplus
}
#endif
#endif /*_LISTVIEW_UTIL_H_INCLUDED_*/
