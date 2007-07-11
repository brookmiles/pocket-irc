#ifndef _DLG_H_INCLUDED_
#define _DLG_H_INCLUDED_

class Dlg
{
public:
	Dlg(HINSTANCE hInst, UINT uID);
	virtual ~Dlg();

	int DoModal(HWND hParent);

protected:
	static BOOL CALLBACK DlgProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual BOOL DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	BOOL DefProc(UINT msg, WPARAM wParam, LPARAM lParam);

	HWND m_hwnd;
	HINSTANCE m_hInst;
	UINT m_uID;
	SHACTIVATEINFO m_shackInfo;
};

#endif//_DLG_H_INCLUDED_
