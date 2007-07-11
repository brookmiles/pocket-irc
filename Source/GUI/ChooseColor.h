#ifndef _CHOOSECOLOR_H_INCLUDED_
#define _CHOOSECOLOR_H_INCLUDED_

bool ChooseColour(CHOOSECOLOR* pcc);
bool ChooseColourMini(HWND hParent, UINT* piColorCode, int xPos, int yPos, LPCTSTR pszTitle = _T("Choose Color"));

#endif//_CHOOSECOLOR_H_INCLUDED_
