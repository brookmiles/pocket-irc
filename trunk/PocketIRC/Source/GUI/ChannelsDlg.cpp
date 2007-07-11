#include "PocketIRC.h"

#include "ChannelsDlg.h"
#include "Config\Options.h"

#include "Common\ListViewUtil.h"
#include "Common\TabCtrlUtil.h"
#include "Common\WindowString.h"
#include "Common\IrcStringFormat.h"

#include "resource.h"


ChannelsDlg::ChannelsDlg(Session* session, Options* pOptions) : Dlg(GetModuleHandle(NULL), IDD_CHANNELS), m_pSession(session), m_pOptions(pOptions)
{
	m_pSession->AddEventHandler(this);
}

ChannelsDlg::~ChannelsDlg()
{
	m_pSession->RemoveEventHandler(this);
}

void ChannelsDlg::OnEvent(const NetworkEvent& networkEvent)
{
	_TRACE("ChannelsDlg(0x%08X)::OnEvent(\"%s\")", this, networkEvent.GetEvent().Str());

	#define HANDLE_IRC_EVENT(id, h) case id: h(networkEvent); break;

	if(IsWindow(m_hwnd))
	{
		if(networkEvent.IsIncoming())
		{
			switch(networkEvent.GetEventID())
			{
				HANDLE_IRC_EVENT(IRC_RPL_LISTSTART, OnRplListStart)
				HANDLE_IRC_EVENT(IRC_RPL_LIST, OnRplList)
				HANDLE_IRC_EVENT(IRC_RPL_LISTEND, OnRplListEnd)
			}
		}
	}
}

void ChannelsDlg::OnRplListStart(const NetworkEvent& event)
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_LISTDLG);
	_ASSERTE(IsWindow(hTab));

	OnListChannelClear();
	EnableWindow(GetDlgItem(hTab, IDC_CHANNELS_LIST_REFRESH), FALSE);
}

void ChannelsDlg::OnRplList(const NetworkEvent& event)
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_LISTDLG);
	_ASSERTE(IsWindow(hTab));

	EnableWindow(GetDlgItem(hTab, IDC_CHANNELS_LIST_REFRESH), FALSE);

	String sChannel = event.GetParam(1);
	String sUsers = event.GetParam(2);
	String sTopic = event.GetParam(3);

	if(sTopic[0] == '[' && sTopic[sTopic.Size() - 1] == ']')
		sTopic = _T("");
	sTopic = StringFormat::StripFormatting(sTopic);

	if(sChannel != _T("*"))
	{
		AddChannelListEntry(sChannel, sUsers, sTopic);
	}
}

void ChannelsDlg::AddChannelListEntry(String& sChannel, String& sUsers, String& sTopic)
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_LISTDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hChanList = GetDlgItem(hTab, IDC_CHANNELS_LIST_LIST);

	UINT minUsers = GetDlgItemInt(hTab, IDC_CHANNELS_LIST_MINUSERS, NULL, FALSE);
	UINT userCount = _tcstoul(sUsers.Str(), NULL, 10);

	if(userCount >= minUsers)
	{
		LVITEM lvi = {LVIF_TEXT};
		lvi.iItem = ListView_GetItemCount(hChanList);
		lvi.iSubItem = 0;
		lvi.pszText = sChannel.Str();

		int index = ListView_InsertItem(hChanList, &lvi);
		_ASSERTE(index != -1);
		if(index != -1)
		{
			ListView_SetItemText(hChanList, index, 1, sUsers.Str());
			ListView_SetItemText(hChanList, index, 2, sTopic.Str());
		}

		int count = ListView_GetItemCount(hChanList);
		SetDlgItemInt(hTab, IDC_CHANNELS_LIST_COUNT, count, FALSE);
	}
}

void ChannelsDlg::OnRplListEnd(const NetworkEvent& event)
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_LISTDLG);
	_ASSERTE(IsWindow(hTab));

	if(m_pSession->IsConnected())
	{
		EnableWindow(GetDlgItem(hTab, IDC_CHANNELS_LIST_REFRESH), TRUE);
	}
}

void ChannelsDlg::RefreshFavouriteChannelsList()
{
	HWND hDlgFavs = GetDlgItem(m_hwnd, IDC_CHANNELS_FAVOURITEDLG);
	_ASSERTE(IsWindow(hDlgFavs));

	FavouriteChannelList& chans = m_pOptions->GetFavouriteChannelList();
	HWND hChanList = GetDlgItem(hDlgFavs, IDC_CHANNELS_FAVOURITES_LIST);

	ListView_DeleteAllItems(hChanList);

	for(UINT iChan = 0; iChan < chans.Count(); ++iChan)
	{
		FavouriteChannel* chan = chans.GetChannel(iChan);
		_ASSERTE(chan != NULL);

		LVITEM lvi = {LVIF_TEXT | LVIF_PARAM};
		lvi.iItem = iChan;
		lvi.iSubItem = 0;
		lvi.pszText = chan->Name.Str();
		lvi.lParam = (LPARAM)chan;

		int index = ListView_InsertItem(hChanList, &lvi);
		_ASSERTE(index != -1);
		ListView_SetItemText(hChanList, index, 1, chan->AutoJoin ? _T("Yes") : _T("No"));
		ListView_SetItemText(hChanList, index, 2, chan->Key.Str());
	}
}

void ChannelsDlg::ActivateTab(bool bActivate)
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_TAB);
	_ASSERTE(IsWindow(hTab));

	int index = TabCtrl_GetCurSel(hTab);
	if(index != -1)
	{
		TCITEM tci = {TCIF_PARAM};
		bool bGetItemOk = TabCtrl_GetItem(hTab, index, &tci) != 0;
		_ASSERTE(bGetItemOk != FALSE);

		HWND hDlg = (HWND)tci.lParam;
		_ASSERTE(IsWindow(hDlg));

		if(bActivate)
		{
			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		}
		else
		{
			ShowWindow(hDlg, SW_HIDE);
		}
	}
}

void ChannelsDlg::FitChildToTab(HWND hChild)
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_TAB);
	_ASSERTE(IsWindow(hTab));

	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);

	RECT rcTab;
	GetWindowRect(hTab, &rcTab);
	MapWindowPoints(NULL, m_hwnd, (LPPOINT)&rcTab, 2);

	TabCtrl_AdjustRect(hTab, FALSE, &rcTab);

	SetWindowPos(hChild, NULL, 0, 0, rcClient.right, rcTab.bottom, SWP_SHOWWINDOW | SWP_NOZORDER);
}

BOOL ChannelsDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLEMSG(WM_INITDIALOG, OnInitDialog);
		HANDLEMSG(WM_COMMAND, OnCommand);
		HANDLEMSGRET(WM_NOTIFY, OnNotify);
		HANDLEMSG(WM_INITMENUPOPUP, OnInitMenuPopup);
		HANDLEMSG(WM_SIZE, OnSize);
		HANDLEMSG(WM_SETTINGCHANGE, OnSettingChange);
		default:
			return Dlg::DefProc(msg, wParam, lParam);
	}
	return TRUE;
}


BOOL CALLBACK ChannelsDlg::ChannelsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_COMMAND:
			// Listview label edit sends WM_COMMAND with IDOK, don't want those
			if(LOWORD(wParam) == IDOK)
				break;
		case WM_NOTIFY:
			return SendMessage(GetParent(hwnd), msg, wParam, lParam);

	}
	return FALSE;
}

BOOL CALLBACK ChannelsDlg::ChannelsFavProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_SIZE:
			UpdateListSize(hwnd, IDC_CHANNELS_FAVOURITES_LIST);
			break;

	}
	return ChannelsProc(hwnd, msg, wParam, lParam);
}

BOOL CALLBACK ChannelsDlg::ChannelsListProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_SIZE:
			UpdateListSize(hwnd, IDC_CHANNELS_LIST_LIST);
			break;
	}
	return ChannelsProc(hwnd, msg, wParam, lParam);
}

void ChannelsDlg::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	Dlg::DefProc(WM_INITDIALOG, wParam, lParam);

	// Set up tab control
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_TAB);
	SendMessage(hTab, CCM_SETVERSION, COMCTL32_VERSION, 0);
	TabCtrl_FitToParent(hTab);

	HWND hDlgFavs = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CHANNELS_FAVOURITES),
		m_hwnd, &ChannelsDlg::ChannelsFavProc); // Don't use dummy proc, listview label edit notifications sends IDOK
	SetWindowLong(hDlgFavs, GWL_ID, IDC_CHANNELS_FAVOURITEDLG);
	FitChildToTab(hDlgFavs);

	HWND hDlgList = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CHANNELS_LIST),
		m_hwnd, &ChannelsDlg::ChannelsListProc); // Don't use dummy proc, listview label edit notifications sends IDOK
	SetWindowLong(hDlgList, GWL_ID, IDC_CHANNELS_LISTDLG);
	FitChildToTab(hDlgList);

	// Add tabs
	TCITEM tci = {TCIF_TEXT | TCIF_PARAM};
	int tabcount = 0;

	tci.pszText = _T("Favourites");
	tci.lParam = (LPARAM)hDlgFavs;
	int iInsertTabOk = TabCtrl_InsertItem(hTab, tabcount++, &tci);
	_ASSERTE(iInsertTabOk != -1);

	tci.pszText = _T("List");
	tci.lParam = (LPARAM)hDlgList;
	iInsertTabOk = TabCtrl_InsertItem(hTab, tabcount++, &tci);
	_ASSERTE(iInsertTabOk != -1);

	// Init Favourites

	// Get listview dimentions
	RECT rcChanList;
	GetClientRect(GetDlgItem(hDlgFavs, IDC_CHANNELS_FAVOURITES_LIST), &rcChanList);

	ListView_SetExtendedListViewStyle(GetDlgItem(hDlgFavs, IDC_CHANNELS_FAVOURITES_LIST), LVS_EX_FULLROWSELECT);

	int columnSpace = rcChanList.right - GetSystemMetrics(SM_CXVSCROLL);
	int cxChannel = (columnSpace * 60) / 100;
	int cxAuto = DRA::SCALEX(40);
	int cxKey = columnSpace - cxChannel - cxAuto;

	// Setup channel list headers
	LVCOLUMN lvc = {LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH};
	lvc.iSubItem = 0;
	lvc.pszText = _T("Channel");
	lvc.cx = cxChannel;
	SendDlgItemMessage(hDlgFavs, IDC_CHANNELS_FAVOURITES_LIST, LVM_INSERTCOLUMN, lvc.iSubItem, (LPARAM)&lvc);

	lvc.iSubItem++;
	lvc.pszText = _T("Auto");
	lvc.cx = cxAuto;
	SendDlgItemMessage(hDlgFavs, IDC_CHANNELS_FAVOURITES_LIST, LVM_INSERTCOLUMN, lvc.iSubItem, (LPARAM)&lvc);

	lvc.iSubItem++;
	lvc.pszText = _T("Key");
	lvc.cx = cxKey;
	SendDlgItemMessage(hDlgFavs, IDC_CHANNELS_FAVOURITES_LIST, LVM_INSERTCOLUMN, lvc.iSubItem, (LPARAM)&lvc);

	RefreshFavouriteChannelsList();

	// Init Server List

	EnableWindow(GetDlgItem(hDlgList, IDC_CHANNELS_LIST_REFRESH), m_pSession->IsConnected());

	// Get listview dimentions
	GetClientRect(GetDlgItem(hDlgList, IDC_CHANNELS_LIST_LIST), &rcChanList);
	SetDlgItemInt(hDlgList, IDC_CHANNELS_LIST_MINUSERS, m_pOptions->GetChanListUserMin(), FALSE);

	ListView_SetExtendedListViewStyle(GetDlgItem(hDlgList, IDC_CHANNELS_LIST_LIST), LVS_EX_FULLROWSELECT);

	columnSpace = rcChanList.right - GetSystemMetrics(SM_CXVSCROLL);
	cxChannel = (columnSpace * 60) / 100;
	int cxUsers = DRA::SCALEX(40);
	cxKey = columnSpace - cxChannel - cxUsers;

	// Setup channel list headers
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.iSubItem = 0;
	lvc.pszText = _T("Channel");
	lvc.cx = cxChannel;
	SendDlgItemMessage(hDlgList, IDC_CHANNELS_LIST_LIST, LVM_INSERTCOLUMN, lvc.iSubItem, (LPARAM)&lvc);

	lvc.iSubItem++;
	lvc.pszText = _T("Users");
	lvc.cx = cxUsers;
	SendDlgItemMessage(hDlgList, IDC_CHANNELS_LIST_LIST, LVM_INSERTCOLUMN, lvc.iSubItem, (LPARAM)&lvc);

	lvc.iSubItem++;
	lvc.pszText = _T("Topic");
	lvc.cx = cxKey;
	SendDlgItemMessage(hDlgList, IDC_CHANNELS_LIST_LIST, LVM_INSERTCOLUMN, lvc.iSubItem, (LPARAM)&lvc);

	const Vector<Session::ChannelListEntry*>& channelList = m_pSession->GetChannelList();
	for(UINT i = 0; i < channelList.Size(); ++i)
	{
		Session::ChannelListEntry* chan = channelList[i];
		_ASSERTE(chan);
		
		AddChannelListEntry(chan->name, chan->users, chan->topic);
	}

	ActivateTab(true);
}

void ChannelsDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case IDOK:
		OnOk();
		break;
	case IDCANCEL:
		EndDialog(m_hwnd, IDCANCEL);
		break;
	case IDC_CHANNELS_FAVOURITES_LIST:
		switch(HIWORD(wParam))
		{
		case LBN_DBLCLK:
			OnOk();
			break;
		}
		break;
	case IDC_CHANNELS_FAVOURITES_SETKEY:
		OnFavouriteChannelSetKey();
		break;
	case ID_FAVOURITECHANNEL_NEW:
		OnFavouriteChannelNew();
		break;
	case ID_FAVOURITECHANNEL_DELETE:
		OnFavouriteChannelDelete();
		break;
	case ID_FAVOURITECHANNEL_RENAME:
		OnFavouriteChannelRename();
		break;
	case ID_FAVOURITECHANNEL_AUTOJOINONCONNECT:
		OnFavouriteChannelAuto();
		break;
	case ID_FAVOURITECHANNEL_JOINNOW:
		OnFavouriteJoinNow();
		break;
	case IDC_CHANNELS_LIST_REFRESH:
		OnListChannelRefresh();
		break;
	case ID_CHANNELLIST_AUTOJOINONCONNECT:
		OnListChannelAuto();
		break;
	case ID_CHANNELLIST_FAVOURITECHANNEL:
		OnListChannelFavourite();
		break;
	case ID_CHANNELLIST_CLEARLIST:
		OnListChannelClear();
		break;
	case ID_CHANNELLIST_JOINNOW:
		OnListChannelJoin();
		break;
	}
}

void ChannelsDlg::OnOk()
{
	HWND hTab = GetDlgItem(m_hwnd, ChannelsDlg::IDC_CHANNELS_LISTDLG);
	_ASSERTE(IsWindow(hTab));

	UINT minUsers = GetDlgItemInt(hTab, IDC_CHANNELS_LIST_MINUSERS, NULL, FALSE);
	m_pOptions->SetChanListUserMin(minUsers);

	EndDialog(m_hwnd, IDOK);
}


LRESULT ChannelsDlg::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* lpnmh = (NMHDR*)lParam;
	switch(lpnmh->idFrom)
	{
	case IDC_CHANNELS_TAB:
		switch(lpnmh->code)
		{
		case TCN_SELCHANGE:
			ActivateTab(true);
			RefreshFavouriteChannelsList();
			break;
		case TCN_SELCHANGING:
			ActivateTab(false);
			break;
		}
		break;
	case IDC_CHANNELS_LIST_LIST:
		{
			switch(lpnmh->code)
			{
			case GN_CONTEXTMENU:
				{
					NMRGINFO* pnmri = (NMRGINFO*)lParam;
					_ASSERTE(pnmri != NULL);

					POINT pt = pnmri->ptAction;
					MapWindowPoints(NULL, lpnmh->hwndFrom, &pt, 1);

					LVHITTESTINFO lvht = {pt};
					int index = ListView_HitTest(lpnmh->hwndFrom, &lvht);

					HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CHANLISTMENU));

					if(index == -1)
					{
						EnableMenuItem(hMenu, ID_CHANNELLIST_AUTOJOINONCONNECT, MF_BYCOMMAND | MF_GRAYED);
						EnableMenuItem(hMenu, ID_CHANNELLIST_FAVOURITECHANNEL, MF_BYCOMMAND | MF_GRAYED);
						EnableMenuItem(hMenu, ID_CHANNELLIST_JOINNOW, MF_BYCOMMAND | MF_GRAYED);
					}
					else
					{
						TCHAR buf[POCKETIRC_MAX_IRC_LINE_LEN] = _T("");
						ListView_GetItemText(lpnmh->hwndFrom, index, 0, buf, sizeof(buf)/sizeof(buf[0]));

						FavouriteChannel* chan = (FavouriteChannel*)m_pOptions->GetFavouriteChannelList().FindChannel(buf);
						if(chan)
						{
							CheckMenuItem(hMenu, ID_CHANNELLIST_FAVOURITECHANNEL, MF_BYCOMMAND | MF_CHECKED);
							CheckMenuItem(hMenu, ID_CHANNELLIST_AUTOJOINONCONNECT, MF_BYCOMMAND | (chan->AutoJoin ? MF_CHECKED : MF_UNCHECKED));
						}
						EnableMenuItem(hMenu, ID_CHANNELLIST_JOINNOW, MF_BYCOMMAND | (m_pSession->IsConnected() ? MF_ENABLED : MF_GRAYED));
					}

					BOOL bTrackPopOk = TrackPopupMenuEx(GetSubMenu(hMenu, 0), TPM_BOTTOMALIGN | TPM_LEFTALIGN, 
						pnmri->ptAction.x, pnmri->ptAction.y, m_hwnd, NULL);
					_ASSERTE(bTrackPopOk);

				}	
				break;
			case LVN_COLUMNCLICK:
				{
					//ListView_SortItems();
				}
				break;
			}
		}
		break;
	case IDC_CHANNELS_FAVOURITES_LIST:
		{
			switch(lpnmh->code)
			{
			case GN_CONTEXTMENU:
				{
					NMRGINFO* pnmri = (NMRGINFO*)lParam;
					_ASSERTE(pnmri != NULL);

					POINT pt = pnmri->ptAction;
					MapWindowPoints(NULL, lpnmh->hwndFrom, &pt, 1);

					LVHITTESTINFO lvht = {pt};
					int index = ListView_HitTest(lpnmh->hwndFrom, &lvht);

					HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_FAVCHANMENU));

					if(index == -1)
					{
						EnableMenuItem(hMenu, ID_FAVOURITECHANNEL_DELETE, MF_BYCOMMAND | MF_GRAYED);
						EnableMenuItem(hMenu, ID_FAVOURITECHANNEL_RENAME, MF_BYCOMMAND | MF_GRAYED);
						EnableMenuItem(hMenu, ID_FAVOURITECHANNEL_AUTOJOINONCONNECT, MF_BYCOMMAND | MF_GRAYED);
						EnableMenuItem(hMenu, ID_FAVOURITECHANNEL_JOINNOW, MF_BYCOMMAND | MF_GRAYED);
					}
					else
					{
						FavouriteChannel* chan = (FavouriteChannel*)ListView_GetItemParam(lpnmh->hwndFrom, index);
						_ASSERTE(chan);

						CheckMenuItem(hMenu, ID_FAVOURITECHANNEL_AUTOJOINONCONNECT, MF_BYCOMMAND | (chan->AutoJoin ? MF_CHECKED : MF_UNCHECKED));
						EnableMenuItem(hMenu, ID_FAVOURITECHANNEL_JOINNOW, MF_BYCOMMAND | (m_pSession->IsConnected() ? MF_ENABLED : MF_GRAYED));
					}

					BOOL bTrackPopOk = TrackPopupMenuEx(GetSubMenu(hMenu, 0), TPM_BOTTOMALIGN | TPM_LEFTALIGN, 
						pnmri->ptAction.x, pnmri->ptAction.y, m_hwnd, NULL);
					_ASSERTE(bTrackPopOk);

				}	
				break;
			case LVN_ITEMCHANGED:
				OnFavouriteChannelSelChange();
				break;
			case LVN_ENDLABELEDIT:
				{
					LV_DISPINFO* lvdi = (LV_DISPINFO*)lParam;

					if(lvdi->item.pszText)
					{
						FavouriteChannel* chan = (FavouriteChannel*)ListView_GetItemParam(lpnmh->hwndFrom, lvdi->item.iItem);

						FavouriteChannelList& list = m_pOptions->GetFavouriteChannelList();
						FavouriteChannel* exist = list.FindChannel(lvdi->item.pszText);
						if(!exist)
						{
							if(!chan)
							{
								chan = list.AddChannel(lvdi->item.pszText, false);
								ListView_SetItemParam(lpnmh->hwndFrom, lvdi->item.iItem, (LPARAM)chan);
								ListView_SetItemText(lpnmh->hwndFrom, lvdi->item.iItem, 1, _T("No"));
							}
							chan->Name = lvdi->item.pszText;
							ListView_SetItemText(lpnmh->hwndFrom, lvdi->item.iItem, 0, lvdi->item.pszText);
						}
						else
						{
							if(!chan)
							{
								// Attempted to create new favourite, but it already existed, delete list view item
								ListView_DeleteItem(lpnmh->hwndFrom, lvdi->item.iItem);
							}
						}
					}
					else
					{
						// User cancelled edit
						FavouriteChannel* chan = (FavouriteChannel*)ListView_GetItemParam(lpnmh->hwndFrom, lvdi->item.iItem);
						if(!chan)
						{
							ListView_DeleteItem(lpnmh->hwndFrom, lvdi->item.iItem);
						}
					}
				}
				break;
			}
		}
		break;
	}
	return FALSE;
}

void ChannelsDlg::OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
}

void ChannelsDlg::OnSize(WPARAM wParam, LPARAM lParam)
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_TAB);
	TabCtrl_FitToParent(hTab);

	FitChildToTab(GetDlgItem(m_hwnd, IDC_CHANNELS_LISTDLG));
	FitChildToTab(GetDlgItem(m_hwnd, IDC_CHANNELS_FAVOURITEDLG));
}

void ChannelsDlg::OnSettingChange(WPARAM wParam, LPARAM lParam)
{
	if(wParam == SPI_SETSIPINFO)
	{
		UpdateSize();
	}
}

void ChannelsDlg::UpdateSize()
{
	SIPINFO si = {sizeof(SIPINFO)};
	SipGetInfo(&si);

	if(!(si.fdwFlags & SIPF_ON))
	{
		HWND hMenuBar = SHFindMenuBar(m_hwnd);
		_ASSERTE(IsWindow(hMenuBar));

		RECT rcMenuBar = {0};
		GetWindowRect(hMenuBar, &rcMenuBar);
		si.rcVisibleDesktop.bottom -= rcMenuBar.bottom - rcMenuBar.top;
	}

	MoveWindow(m_hwnd, si.rcVisibleDesktop.left, si.rcVisibleDesktop.top,
		si.rcVisibleDesktop.right - si.rcVisibleDesktop.left,
		si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top,
		FALSE);

}

void ChannelsDlg::UpdateListSize(HWND hDlg, UINT listId)
{
	HWND hList = GetDlgItem(hDlg, listId);
	_ASSERTE(IsWindow(hList));

	RECT listRect;
	GetWindowRect(hList, &listRect);
	MapWindowPoints(HWND_DESKTOP, hDlg, (POINT*)&listRect, 2);

	RECT dlgRect;
	GetClientRect(hDlg, &dlgRect);

	SetWindowPos(hList, NULL, 0, 0, dlgRect.right - (listRect.left * 2), dlgRect.bottom - listRect.top - listRect.left, SWP_NOMOVE | SWP_NOZORDER);
}

void ChannelsDlg::OnFavouriteChannelNew()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_FAVOURITEDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_FAVOURITES_LIST);
	_ASSERTE(IsWindow(hList));

	LVITEM lvi = {LVIF_TEXT};
	lvi.iItem = ListView_GetItemCount(hList);
	lvi.iSubItem = 0;
	lvi.pszText = _T("#");
	int index = ListView_InsertItem(hList, &lvi);

	if(index <= 0)
	{
		ListView_EnsureVisible(hList, index, FALSE);
		ListView_EditLabel(hList, index);
	}
}

void ChannelsDlg::OnFavouriteChannelDelete()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_FAVOURITEDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_FAVOURITES_LIST);
	_ASSERTE(IsWindow(hList));

	int count = ListView_GetItemCount(hList);
	for(int i = count - 1; i >= 0; --i)
	{
		if(ListView_GetItemState(hList, i, LVIS_SELECTED) & LVIS_SELECTED)
		{
			FavouriteChannel* chan = (FavouriteChannel*)ListView_GetItemParam(hList, i);
			_ASSERTE(chan);

			ListView_DeleteItem(hList, i);
			m_pOptions->GetFavouriteChannelList().RemoveChannel(chan);
		}
	}
}

void ChannelsDlg::OnFavouriteChannelRename()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_FAVOURITEDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_FAVOURITES_LIST);
	_ASSERTE(IsWindow(hList));

	int count = ListView_GetItemCount(hList);
	for(int i = 0; i < count; ++i)
	{
		if(ListView_GetItemState(hList, i, LVIS_FOCUSED) & LVIS_FOCUSED)
		{
			ListView_EditLabel(hList, i);
			return;
		}
	}
}

void ChannelsDlg::OnFavouriteChannelAuto()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_FAVOURITEDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_FAVOURITES_LIST);
	_ASSERTE(IsWindow(hList));

	bool curAuto = false;

	int count = ListView_GetItemCount(hList);
	for(int i = 0; i < count; ++i)
	{
		if(ListView_GetItemState(hList, i, LVIS_FOCUSED) & LVIS_FOCUSED)
		{
			FavouriteChannel* chan = (FavouriteChannel*)ListView_GetItemParam(hList, i);
			_ASSERTE(chan);

			curAuto = chan->AutoJoin;

			break;
		}
	}

	for(int i = 0; i < count; ++i)
	{
		if(ListView_GetItemState(hList, i, LVIS_SELECTED) & LVIS_SELECTED)
		{
			FavouriteChannel* chan = (FavouriteChannel*)ListView_GetItemParam(hList, i);
			_ASSERTE(chan);

			chan->AutoJoin = !curAuto;

			ListView_SetItemText(hList, i, 1, chan->AutoJoin ? _T("Yes") : _T("No"));
		}
	}
}

void ChannelsDlg::OnFavouriteChannelSelChange()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_FAVOURITEDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_FAVOURITES_LIST);
	_ASSERTE(IsWindow(hList));

	String sKey;

	int count = ListView_GetItemCount(hList);
	for(int i = 0; i < count; ++i)
	{
		if((ListView_GetItemState(hList, i, LVIS_FOCUSED) & LVIS_FOCUSED) && (ListView_GetItemState(hList, i, LVIS_SELECTED) & LVIS_SELECTED))
		{
			FavouriteChannel* chan = (FavouriteChannel*)ListView_GetItemParam(hList, i);
			if(chan)
			{
				sKey = chan->Key;
			}
			break;
		}
	}

	SetDlgItemText(hTab, IDC_CHANNELS_FAVOURITES_KEY, sKey.Str());
}

void ChannelsDlg::OnFavouriteChannelSetKey()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_FAVOURITEDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_FAVOURITES_LIST);
	_ASSERTE(IsWindow(hList));

	String sKey;
	GetDlgItemString(hTab, IDC_CHANNELS_FAVOURITES_KEY, sKey);

	int count = ListView_GetItemCount(hList);
	for(int i = 0; i < count; ++i)
	{
		if(ListView_GetItemState(hList, i, LVIS_SELECTED) & LVIS_SELECTED)
		{
			FavouriteChannel* chan = (FavouriteChannel*)ListView_GetItemParam(hList, i);
			_ASSERTE(chan);

			chan->Key = sKey;

			ListView_SetItemText(hList, i, 2, chan->Key.Str());
		}
	}
}

void ChannelsDlg::OnFavouriteJoinNow()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_FAVOURITEDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_FAVOURITES_LIST);
	_ASSERTE(IsWindow(hList));

	String sKey;
	GetDlgItemString(hTab, IDC_CHANNELS_FAVOURITES_KEY, sKey);

	bool bAnyJoined = false;

	int count = ListView_GetItemCount(hList);
	for(int i = 0; i < count; ++i)
	{
		if(ListView_GetItemState(hList, i, LVIS_SELECTED) & LVIS_SELECTED)
		{
			FavouriteChannel* chan = (FavouriteChannel*)ListView_GetItemParam(hList, i);
			_ASSERTE(chan);

			m_pSession->Join(chan->Name, chan->Key);
			bAnyJoined = true;
		}
	}
	if(bAnyJoined)
	{
		EndDialog(m_hwnd, IDOK);
	}
}

void ChannelsDlg::OnListChannelRefresh()
{
	m_pSession->List();
}

void ChannelsDlg::OnListChannelAuto()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_LISTDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_LIST_LIST);
	_ASSERTE(IsWindow(hList));

	String sKey;
	// Removed for space considerations, want to join with a key?  use /join
	//GetDlgItemString(hTab, IDC_CHANNELS_LIST_KEY, sKey); 

	bool curAuto = false;

	int count = ListView_GetItemCount(hList);
	for(int i = 0; i < count; ++i)
	{
		if(ListView_GetItemState(hList, i, LVIS_FOCUSED) & LVIS_FOCUSED)
		{
			String sChannel;
			sChannel.Reserve(POCKETIRC_MAX_IRC_LINE_LEN);

			ListView_GetItemText(hList, i, 0, sChannel.Str(), sChannel.Capacity());
			_ASSERTE(sChannel.Size() > 0);

			if(sChannel.Size() > 0)
			{
				FavouriteChannel* chan = m_pOptions->GetFavouriteChannelList().FindChannel(sChannel);
				if(chan)
				{
					curAuto = chan->AutoJoin;
				}
				else
				{
					curAuto = false;
				}
			}

			break;
		}
	}

	for(int i = 0; i < count; ++i)
	{
		if(ListView_GetItemState(hList, i, LVIS_SELECTED) & LVIS_SELECTED)
		{
			String sChannel;
			sChannel.Reserve(POCKETIRC_MAX_IRC_LINE_LEN);

			ListView_GetItemText(hList, i, 0, sChannel.Str(), sChannel.Capacity());
			_ASSERTE(sChannel.Size() > 0);

			if(sChannel.Size() > 0)
			{
				FavouriteChannel* chan = m_pOptions->GetFavouriteChannelList().FindChannel(sChannel);
				if(chan)
				{
					chan->AutoJoin = !curAuto;
				}
				else
				{
					if(!curAuto)
					{
						m_pOptions->GetFavouriteChannelList().AddChannel(sChannel, sKey, true);
					}
				}
			}
		}
	}
}

void ChannelsDlg::OnListChannelFavourite()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_LISTDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_LIST_LIST);
	_ASSERTE(IsWindow(hList));

	String sKey;
	//GetDlgItemString(hTab, IDC_CHANNELS_LIST_KEY, sKey);

	bool curFav = false;

	int count = ListView_GetItemCount(hList);
	for(int i = 0; i < count; ++i)
	{
		if(ListView_GetItemState(hList, i, LVIS_FOCUSED) & LVIS_FOCUSED)
		{
			String sChannel;
			sChannel.Reserve(POCKETIRC_MAX_IRC_LINE_LEN);

			ListView_GetItemText(hList, i, 0, sChannel.Str(), sChannel.Capacity());
			_ASSERTE(sChannel.Size() > 0);

			if(sChannel.Size() > 0)
			{
				FavouriteChannel* chan = m_pOptions->GetFavouriteChannelList().FindChannel(sChannel);
				curFav = (chan != NULL);
			}

			break;
		}
	}

	for(int i = 0; i < count; ++i)
	{
		if(ListView_GetItemState(hList, i, LVIS_SELECTED) & LVIS_SELECTED)
		{
			String sChannel;
			sChannel.Reserve(POCKETIRC_MAX_IRC_LINE_LEN);

			ListView_GetItemText(hList, i, 0, sChannel.Str(), sChannel.Capacity());
			_ASSERTE(sChannel.Size() > 0);

			if(sChannel.Size() > 0)
			{
				FavouriteChannel* chan = m_pOptions->GetFavouriteChannelList().FindChannel(sChannel);
				if(chan)
				{
					if(curFav)
					{
						m_pOptions->GetFavouriteChannelList().RemoveChannel(sChannel);
					}
				}
				else
				{
					if(!curFav)
					{
						m_pOptions->GetFavouriteChannelList().AddChannel(sChannel, sKey);
					}
				}
			}
		}
	}
}

void ChannelsDlg::OnListChannelClear()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_LISTDLG);
	_ASSERTE(IsWindow(hTab));
	SetDlgItemInt(hTab, IDC_CHANNELS_LIST_COUNT, 0, FALSE);

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_LIST_LIST);
	_ASSERTE(IsWindow(hList));
	ListView_DeleteAllItems(hList);

	m_pSession->ClearChannelList();
}

void ChannelsDlg::OnListChannelJoin()
{
	HWND hTab = GetDlgItem(m_hwnd, IDC_CHANNELS_LISTDLG);
	_ASSERTE(IsWindow(hTab));

	HWND hList = GetDlgItem(hTab, IDC_CHANNELS_LIST_LIST);
	_ASSERTE(IsWindow(hList));

	String sKey;
	//GetDlgItemString(hTab, IDC_CHANNELS_LIST_KEY, sKey);

	bool bAnyJoined = false;

	int count = ListView_GetItemCount(hList);
	for(int i = 0; i < count; ++i)
	{
		if(ListView_GetItemState(hList, i, LVIS_SELECTED) & LVIS_SELECTED)
		{
			String sChannel;
			sChannel.Reserve(POCKETIRC_MAX_IRC_LINE_LEN);

			ListView_GetItemText(hList, i, 0, sChannel.Str(), sChannel.Capacity());
			_ASSERTE(sChannel.Size() > 0);

			if(sChannel.Size() > 0)
			{
				FavouriteChannel* chan = m_pOptions->GetFavouriteChannelList().FindChannel(sChannel);
				if(chan)
				{
					m_pSession->Join(chan->Name, chan->Key);
				}
				else
				{
					m_pSession->Join(sChannel, sKey);
				}
				bAnyJoined = true;
			}
		}
	}

	if(bAnyJoined)
	{
		EndDialog(m_hwnd, IDOK);
	}
}
