#ifndef _OPTIONDLG_H_INCLUDED_
#define _OPTIONDLG_H_INCLUDED_

#include "Dlg.h"
#include "Config\Options.h"

class OptionDlg : public Dlg
{
	enum IDC_OPTIONS_ENUM {
		IDC_OPTIONS_SERVERDLG = 201,
		IDC_OPTIONS_DISPLAYDLG,
		IDC_OPTIONS_FORMATDLG,
		IDC_OPTIONS_IDENTDLG,
		IDC_OPTIONS_DCCDLG,
		IDC_OPTIONS_ABOUTDLG
	};
public:
	OptionDlg(Options* pOptions);
	~OptionDlg();

private:
	static int CALLBACK EnumFontFamStub(ENUMLOGFONT FAR *lpelf, TEXTMETRIC FAR *lpntm, int FontType, LPARAM lParam);
	int CALLBACK EnumFontFamProc(ENUMLOGFONT FAR *lpelf, TEXTMETRIC FAR *lpntm, int FontType);

	static BOOL CALLBACK DummyProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
	
	void OnInitDialog(WPARAM wParam, LPARAM lParam);
	void OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
	void OnSize(WPARAM wParam, LPARAM lParam);
	void OnDrawItem(WPARAM wParam, LPARAM lParam);
	void OnVScroll(WPARAM wParam, LPARAM lParam);
	void OnSettingChange(WPARAM wParam, LPARAM lParam);

	void OnOk();
	void OnRemoveServer();
	void OnServerNameSelEndOk();
	void OnFormatSelEndOk();
	void OnFormatEnable();
	void OnFormatStringChange();
	void OnDisplayBrowse();
	void OnFormatReset();
	void OnFormatResetAll();
	void OnFormatTextColor();
	void OnFormatBackColor();
	void OnFavouriteChannelNew();
	void OnFavouriteChannelDelete();
	void OnFavouriteChannelRename();
	void OnFavouriteChannelAuto();
	void OnFavouriteChannelSelChange();
	void OnFavouriteChannelSetKey();

	Format* GetCurFormatSel();
	void ActivateTab(bool bActivate);
	HWND GetDlg(UINT idDlg);
	void UpdateSize();

	Options* m_pOptions;
	HWND m_hwndPane;
};
	
#endif//_OPTIONDLG_H_INCLUDED_
