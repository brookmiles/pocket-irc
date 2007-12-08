#ifndef _CHANNELSDLG_H_INCLUDED_
#define _CHANNELSDLG_H_INCLUDED_

#include "Dlg.h"
#include "Core\Session.h"
#include "Config\Options.h"

class ChannelsDlg : 
	public Dlg, 
	public INetworkEventNotify
{
	enum IDC_OPTIONS_ENUM {
		IDC_CHANNELS_FAVOURITEDLG = 201,
		IDC_CHANNELS_LISTDLG,
	};
public:
	ChannelsDlg(Session* session, Options* pOptions);
	~ChannelsDlg();

	void OnEvent(const NetworkEvent& event);

private:
	static BOOL CALLBACK ChannelsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK ChannelsFavProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK ChannelsListProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void OnInitDialog(WPARAM wParam, LPARAM lParam);
	void OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
	void OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
	void OnSize(WPARAM wParam, LPARAM lParam);
	void OnSettingChange(WPARAM wParam, LPARAM lParam);

	void OnOk();

	void OnFavouriteChannelSetKey();
	void OnFavouriteChannelNew();
	void OnFavouriteChannelDelete();
	void OnFavouriteChannelRename();
	void OnFavouriteChannelAuto();
	void OnFavouriteChannelSelChange();
	void OnFavouriteJoinNow();

	void OnRplListStart(const NetworkEvent& event);
	void OnRplList(const NetworkEvent& event);
	void OnRplListEnd(const NetworkEvent& event);

	void OnListChannelRefresh();
	void OnListChannelAuto();
	void OnListChannelFavourite();
	void OnListChannelClear();
	void OnListChannelJoin();

	Session* m_pSession;
	Options* m_pOptions;

	void RefreshFavouriteChannelsList();
	void ActivateTab(bool bActivate);
	void FitChildToTab(HWND hChild);
	void UpdateSize();
	void AddChannelListEntry(tstring& sChannel, tstring& sUsers, tstring& sTopic);

	static void UpdateListSize(HWND hDlg, UINT listId);
};

#endif//_CHANNELSDLG_H_INCLUDED_
