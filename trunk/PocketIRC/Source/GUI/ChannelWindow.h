#ifndef _CHANNELWINDOW_H_INCLUDED_
#define _CHANNELWINDOW_H_INCLUDED_

#include "Core\Channel.h"

#include "DisplayWindow.h"

#define CHANNELWINDOW_NICKLIST_WIDTH 80

class ChannelWindow : 
	public DisplayWindow
{
public:
	DECL_WINDOW_CLASSNAME("PocketIRCChannelWnd");

	enum CHANNEL_IDC { IDC_NICKLIST = 200 };

	ChannelWindow();
	~ChannelWindow();

	void SetChannel(const tstring& sChannel);
	bool HasUser(const tstring& sUser);

// IDisplayWindow
	void OnEvent(const NetworkEvent& networkEvent);

// ITabWindow (partially implemented by DisplayWindow)
	void DoMenu(POINT pt);
	TABWINDOWTYPE GetTabType() { return TABWINDOWTYPE_CHANNEL; };

private:
	static LRESULT NickListSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	void OnDestroy(WPARAM wParam, LPARAM lParam);
	void OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnCtlColorListBox(WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

	void OnJoin(const NetworkEvent& event);
	void OnPart(const NetworkEvent& event);
	void OnKick(const NetworkEvent& event);
	void OnNick(const NetworkEvent& event);
	void OnQuit(const NetworkEvent& event);
	void OnMode(const NetworkEvent& event);
	void OnRplNamReply(const NetworkEvent& event);
	void OnRplEndOfNames(const NetworkEvent& event);
	void OnConnectStateChange(const NetworkEvent &e);

	int GetNickListIndex(const tstring& sUser);
	tstring GetNickListEntry(int index);
	void DoUserOnChannelMenu(WORD x, WORD y, const tstring& sUser);
	bool OnTabMenuCommand(UINT idCmd);

	void OnUserAdd(const tstring& sUser, bool bOp, bool bVoice);
	void OnUserRemove(const tstring& sUser);
	void OnUserUpdate(const tstring& sUser, const tstring& sNewNick = _T(""));

	tstring m_sChannel;
	HWND m_hNickList;
	bool m_bShowNickList;
	bool m_bHaveNickList;
	bool m_bOnChannel;
};

#endif//_CHANNELWINDOW_H_INCLUDED_
