#include "PocketIRC.h"
#include "ChannelWindow.h"

#include "IrcString.h"
#include "StringUtil.h"
#include "Config\Options.h"

#include "resource.h"

LRESULT ChannelWindow::NickListSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_LBUTTONDOWN:
		{
			long ret = CallWindowProc((WNDPROC)GetWindowLong(hwnd, GWL_USERDATA), hwnd, msg, wParam, lParam);

			SHRGINFO srgi = {sizeof(SHRGINFO), hwnd, {LOWORD(lParam), HIWORD(lParam)}, SHRG_NOTIFYPARENT};
			SHRecognizeGesture(&srgi);

			return ret;
		}
		break;
	}
	return CallWindowProc((WNDPROC)GetWindowLong(hwnd, GWL_USERDATA), hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
//	IDisplayWindowFactory
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

ChannelWindow::ChannelWindow()
{
	_TRACE("ChannelWindow(0x%08X)::ChannelWindow()", this);

	m_hNickList = NULL;
	m_bShowNickList = false;
	m_bHaveNickList = false;
	m_bOnChannel = false;
}

ChannelWindow::~ChannelWindow()
{
	_TRACE("ChannelWindow(0x%08X)::~ChannelWindow()", this);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Creation
/////////////////////////////////////////////////////////////////////////////

void ChannelWindow::SetChannel(const tstring& sChannel)
{
	_TRACE("ChannelWindow(0x%08X)::SetChannel(\"%s\")", this, m_sChannel.c_str());
	m_sChannel = sChannel;

	Channel* chan = m_pMainWindow->GetSession()->GetChannel(m_sChannel);
	_ASSERTE(chan);

	SetKey(m_sChannel);
	SetTitle(m_sChannel);
}

/////////////////////////////////////////////////////////////////////////////
//	IDisplayWindow
/////////////////////////////////////////////////////////////////////////////


void ChannelWindow::OnJoin(const NetworkEvent& event)
{
	_TRACE("ChannelWindow(0x%08X)::OnJoin()", this);
	tstring sUser = GetPrefixNick(event.GetPrefix());

	if(m_pMainWindow->GetSession()->IsMe(sUser))
	{
		// Don't add to channel list, it will come with names list
		m_bOnChannel = true;
	}
	else
	{
		OnUserAdd(sUser);
	}
}

void ChannelWindow::OnPart(const NetworkEvent& event)
{
	_TRACE("ChannelWindow(0x%08X)::OnPart()", this);
	tstring sUser = GetPrefixNick(event.GetPrefix());

	if(event.IsIncoming())
	{
		OnUserRemove(sUser);
	}
	else
	{
		Close();
	}
}

void ChannelWindow::OnKick(const NetworkEvent& event)
{
	_TRACE("ChannelWindow(0x%08X)::OnKick()", this);
	const tstring& sUser = event.GetParam(1);

	OnUserRemove(sUser);
}

void ChannelWindow::OnNick(const NetworkEvent& event)
{
	_TRACE("ChannelWindow(0x%08X)::OnNick()", this);
	tstring sUser = GetPrefixNick(event.GetPrefix());
	const tstring& sNewNick = event.GetParam(0);

	OnUserUpdate(sUser, sNewNick);
}

void ChannelWindow::OnMode(const NetworkEvent& event)
{
	_TRACE("ChannelWindow(0x%08X)::OnMode()", this);

	std::vector<ChannelMode> modeList;
	ParseChannelModes(event, modeList);

	for(UINT i = 0; i < modeList.size(); ++i)
	{
		ChannelMode& mode = modeList[i];
		switch(mode.mode)
		{
		case 'v':
		case 'o':
			{
				tstring& sUser = mode.param;
				_ASSERTE(sUser.size());

				if(sUser.size())
				{
					OnUserUpdate(sUser);
				}
			}
			break;
		}
	}
}

void ChannelWindow::OnQuit(const NetworkEvent& event)
{
	_TRACE("ChannelWindow(0x%08X)::OnQuit()", this);
	tstring sUser = GetPrefixNick(event.GetPrefix());

	OnUserRemove(sUser);
}

void ChannelWindow::OnConnectStateChange(const NetworkEvent &e)
{
	bool connected = (e.GetEventID() == SYS_EVENT_CONNECTED);
	_TRACE("ChannelWindow(0x%08X)::OnConnectStateChange(connected == %s)", this, connected ? _T("true") : _T("false"));

	if(!connected)
	{
		OnUserRemove(m_pMainWindow->GetSession()->GetNick());
	}
}

void ChannelWindow::OnRplNamReply(const NetworkEvent& event)
{
	_TRACE("ChannelWindow(0x%08X)::OnRplNamReply()", this);

	const tstring& sNames = event.GetParam(3);
	_ASSERTE(sNames.size());

	// Don't do this on any old NAMES reply, only the first one on join
	if(m_bOnChannel && !m_bHaveNickList)
	{
		UINT iName = 0;
		tstring sName;

		while((sName = GetWord(sNames, iName)).size() > 0)
		{
			OnUserAdd(sName);
			iName++;
		}
	}
}

void ChannelWindow::OnRplEndOfNames(const NetworkEvent& event)
{
	_TRACE("ChannelWindow(0x%08X)::OnRplEndOfNames()", this);

	m_bHaveNickList = true;
}

void ChannelWindow::OnEvent(const NetworkEvent& networkEvent)
{
	_TRACE("MainWindow(0x%08X)::OnEvent(\"%s\")", this, networkEvent.GetEvent().c_str());

	#define HANDLE_IRC_EVENT(id, h) case id: h(networkEvent); break;
	#define HANDLE_IRC_EVENT_RETURN(id, h) case id: h(networkEvent); return;

	if(networkEvent.IsIncoming())
	{
		switch(networkEvent.GetEventID())
		{
			HANDLE_IRC_EVENT(IRC_CMD_NICK, OnNick)
			HANDLE_IRC_EVENT(IRC_CMD_JOIN, OnJoin)
			HANDLE_IRC_EVENT(IRC_CMD_PART, OnPart)
			HANDLE_IRC_EVENT(IRC_CMD_KICK, OnKick)
			HANDLE_IRC_EVENT(IRC_CMD_QUIT, OnQuit)
			HANDLE_IRC_EVENT(IRC_CMD_MODE, OnMode)

			HANDLE_IRC_EVENT_RETURN(IRC_RPL_NAMREPLY, OnRplNamReply)
			HANDLE_IRC_EVENT_RETURN(IRC_RPL_ENDOFNAMES, OnRplEndOfNames)
		}
	}

	switch(networkEvent.GetEventID())
	{
		HANDLE_IRC_EVENT(IRC_CMD_PART, OnPart)
		HANDLE_IRC_EVENT(SYS_EVENT_CONNECTED, OnConnectStateChange)
		HANDLE_IRC_EVENT(SYS_EVENT_CLOSE, OnConnectStateChange)
	}

	DisplayWindow::OnEvent(networkEvent);
}

/////////////////////////////////////////////////////////////////////////////
//	Window Procedure
/////////////////////////////////////////////////////////////////////////////

LRESULT ChannelWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLEMSGRET(WM_CREATE, OnCreate);
		HANDLEMSG(WM_DESTROY, OnDestroy);
		HANDLEMSG(WM_SIZE, OnSize);
		HANDLEMSGRET(WM_CTLCOLORLISTBOX, OnCtlColorListBox);
		HANDLEMSGRET(WM_NOTIFY, OnNotify);
		default:
			return DisplayWindow::WndProc(msg, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//	Message Handling
/////////////////////////////////////////////////////////////////////////////

LRESULT ChannelWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
	_TRACE("ChannelWindow(0x%08X)::OnCreate()", this);
	DisplayWindow::OnCreate(wParam, lParam);

	m_hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CHANNELMENU));


	m_hNickList = CreateWindowEx(0, _T("LISTBOX"), _T("Nickname List"), 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_SORT | LBS_NOINTEGRALHEIGHT, 0, 0, 50, 240, 
		m_hwnd, (HMENU)IDC_NICKLIST, GetModuleHandle(NULL), NULL);
	_ASSERTE(IsWindow(m_hNickList));

	SetWindowLong(m_hNickList, GWL_USERDATA, GetWindowLong(m_hNickList, GWL_WNDPROC));
	SetWindowLong(m_hNickList, GWL_WNDPROC, (LONG)&ChannelWindow::NickListSubProc);

	SendMessage(m_hNickList, WM_SETFONT, (WPARAM)g_Options.GetControlFont(), 0);

	return 0;
}

void ChannelWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	DestroyMenu(m_hMenu);
	m_hMenu = NULL;

	DisplayWindow::OnDestroy(wParam, lParam);
}


void ChannelWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	_TRACE("ChannelWindow(0x%08X)::OnSize()", this);

	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);

	if(m_bShowNickList)
	{
		ShowWindow(m_hNickList, SW_SHOWNORMAL);
		MoveWindow(m_hNickList, rcClient.right - DRA::SCALEX(CHANNELWINDOW_NICKLIST_WIDTH), 0, DRA::SCALEX(CHANNELWINDOW_NICKLIST_WIDTH), rcClient.bottom, TRUE);

		MoveWindow(m_View.GetWindow(), 0, 0, rcClient.right - DRA::SCALEX(CHANNELWINDOW_NICKLIST_WIDTH), rcClient.bottom, TRUE);
	}
	else
	{
		ShowWindow(m_hNickList, SW_HIDE);

		MoveWindow(m_View.GetWindow(), 0, 0, rcClient.right, rcClient.bottom, TRUE);
	}
}

LRESULT ChannelWindow::OnCtlColorListBox(WPARAM wParam, LPARAM lParam)
{
	HDC hdc = (HDC)wParam;

	SetTextColor(hdc, g_Options.GetTextColor());
	SetBkColor(hdc, g_Options.GetBackColor());

	return (LRESULT)g_Options.GetBackColorBrush();
}

LRESULT ChannelWindow::OnNotify(WPARAM wParam, LPARAM lParam)
{
	_TRACE("ChannelWindow(0x%08X)::OnNotify()", this);
	NMHDR* pnmh = (NMHDR*)lParam;
	switch(pnmh->code)
	{
		case GN_CONTEXTMENU:
		{
			NMRGINFO* pnmri = (NMRGINFO*)lParam;
			_ASSERTE(pnmri != NULL);

			_TRACE("...GN_CONTEXTMENU ID(%d)", pnmri->hdr.idFrom);

			switch(pnmri->hdr.idFrom)
			{
				case IDC_VIEW:
				{
					_TRACE("...IDC_VIEW");
					
					POINT pt = pnmri->ptAction;
					MapWindowPoints(NULL, m_View.GetWindow(), &pt, 1);

					UINT iMsg;
					UINT iChar;
					if(m_View.HitTestMsg((WORD)pt.x, (WORD)pt.y, &iMsg, &iChar))
					{
						tstring str = m_View.GetWordAtChar(iMsg, iChar);
						_TRACE("...Word selected: \"%s\"", str.c_str());

						// Modify pt in place, map to screen coords
						MapWindowPoints(m_hwnd, NULL, &pt, 1);

						tstring sNick = StripNick(str, GetUserModes());
						int index = GetNickListIndex(sNick);
						if(index >= 0)
						{
							DoUserOnChannelMenu((WORD)pt.x, (WORD)pt.y, sNick);
						}
						else if(IsChannelString(str))
						{
							DoOffChannelMenu((WORD)pt.x, (WORD)pt.y, str);
						}
						else if(IsUrl(str))
						{
							DoUrlMenu((WORD)pt.x, (WORD)pt.y, str);
						}
						else
						{
							DoMenu(pt);
						}
					}
					else
					{
						DoMenu(pt);
					}
				}
				return TRUE;
				case IDC_NICKLIST:
				{
					_TRACE("...IDC_NICKLIST");

					int index = SendMessage(pnmri->hdr.hwndFrom, LB_GETCURSEL, 0, 0);
					_TRACE("...item (%d)", index);

					if(index != LB_ERR)
					{
						tstring sUser = GetNickListEntry(index);
						_ASSERTE(sUser.size() > 0);

						DoUserOnChannelMenu((WORD)pnmri->ptAction.x, (WORD)pnmri->ptAction.y, sUser);
					}
				}
				return TRUE;
			}
		}	
		break;
	}

	return DisplayWindow::OnNotify(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
//	IChannelUINotify
/////////////////////////////////////////////////////////////////////////////

void ChannelWindow::OnUserAdd(const tstring& sNewUser)
{
	_TRACE("ChannelWindow(0x%08X)::OnUserAdd(%s)", this, sNewUser.c_str());

	tstring sUser = StripNick(sNewUser, GetUserModes());

	if(m_pMainWindow->GetSession()->IsMe(sUser))
	{
		m_bOnChannel = true;
	}

	int index = SendMessage(m_hNickList, LB_ADDSTRING, 0, (LPARAM)sNewUser.c_str());
	_ASSERTE(index != LB_ERR);
	//if(index != LB_ERR)
	//{
	//	SendMessage(m_hNickList, LB_SETITEMDATA, index, (LPARAM)((bOp || bVoice) ? 1 : 0));
	//}
}

void ChannelWindow::OnUserRemove(const tstring& sUser)
{
	_TRACE("ChannelWindow(0x%08X)::OnUserRemove(%s)", this, sUser.c_str());

	if(m_pMainWindow->GetSession()->IsMe(sUser))
	{
		SendMessage(m_hNickList, LB_RESETCONTENT, 0, 0);
		m_bHaveNickList = false;
		m_bOnChannel = false;
	}
	else
	{
		int index = GetNickListIndex(sUser);
		if(index >= 0)
		{
			SendMessage(m_hNickList, LB_DELETESTRING, index, 0);
		}
	}
}

tstring ChannelWindow::PrependUserMode(const tstring& sUser)
{
	Channel* chan = m_pMainWindow->GetSession()->GetChannel(m_sChannel);
	_ASSERTE(chan);
	if(chan)
	{
		return chan->GetHighestUserMode(sUser) + sUser;
	}

	return sUser;
}

void ChannelWindow::OnUserUpdate(const tstring& sUser, const tstring& sNewNick)
{
	_TRACE("ChannelWindow(0x%08X)::OnUserUpdate(%s, %s)", this, sUser.c_str(), sNewNick.c_str());

	tstring sCurrentNick = sNewNick.size() > 0 ? sNewNick : sUser;

	int index = GetNickListIndex(sUser);
	_ASSERTE(index >= 0);
	if(index >= 0)
	{
		SendMessage(m_hNickList, LB_DELETESTRING, index, 0);
	}

	tstring sDisplay = PrependUserMode(sCurrentNick);

	index = SendMessage(m_hNickList, LB_ADDSTRING, 0, (LPARAM)sDisplay.c_str());
	_ASSERTE(index != LB_ERR);
	//if(index != LB_ERR)
	//{
	//	SendMessage(m_hNickList, LB_SETITEMDATA, index, (LPARAM)((chan->IsOp(sCurrentNick) || chan->IsVoice(sCurrentNick)) ? 1 : 0));
	//}
}

/////////////////////////////////////////////////////////////////////////////
//	ITabWindow
/////////////////////////////////////////////////////////////////////////////

bool ChannelWindow::OnTabMenuCommand(UINT idCmd)
{
	_TRACE("ChannelWindow(0x%08X)::OnTabMenuCommand(%u)", this, idCmd);

	tstring sInput = m_pMainWindow->GetInput();

	switch(idCmd)
	{
		case ID_SAY:
		case ID_CHANNEL_SAY:
		{
			if(sInput.size())
			{
				m_pMainWindow->GetSession()->PrivMsg(m_sChannel, sInput);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_CHANNEL_ACT:
		{
			if(sInput.size())
			{
				m_pMainWindow->GetSession()->Action(m_sChannel, sInput);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_CHANNEL_NOTICE:
		{
			if(sInput.size())
			{
				m_pMainWindow->GetSession()->Notice(m_sChannel, sInput);
				m_pMainWindow->ClearInput();
			}
		}
		break;
		case ID_CHANNEL_TOPIC:
		{
			m_pMainWindow->GetSession()->Topic(m_sChannel, sInput);
			m_pMainWindow->ClearInput();
		}
		break;
		case ID_CHANNEL_INVITE:
		{
			m_pMainWindow->GetSession()->Invite(sInput, m_sChannel);
			m_pMainWindow->ClearInput();
		}
		break;
		case ID_CHANNEL_MODE:	
		{
			m_pMainWindow->GetSession()->Mode(m_sChannel, sInput);
			m_pMainWindow->ClearInput();
		}
		break;
		case ID_CHANNEL_PART:
		{
			m_pMainWindow->GetSession()->Part(m_sChannel);
		}
		break;
		case ID_CHANNEL_REJOIN:
		{
			if(sInput.size() == 0)
			{
				FavouriteChannel* pFavChan = g_Options.GetFavouriteChannelList().FindChannel(m_sChannel);
				if(pFavChan)
				{
					sInput = pFavChan->Key;
				}
			}
			m_pMainWindow->GetSession()->Join(m_sChannel, sInput);
			m_pMainWindow->ClearInput();
		}
		break;
		case ID_CHANNEL_SHOWUSERLIST:
		{
			m_bShowNickList = !m_bShowNickList;
			OnSize(0, 0);
		}
		break;
		case ID_CHANNEL_FAVOURITECHANNEL:
		{
			FavouriteChannel* chan = g_Options.GetFavouriteChannelList().FindChannel(m_sChannel);
			if(chan)
			{
				g_Options.GetFavouriteChannelList().RemoveChannel(chan);
			}
			else
			{
				g_Options.GetFavouriteChannelList().AddChannel(m_sChannel);
			}
		}
		break;
		case ID_CHANNEL_AUTOJOINONCONNECT:
		{
			FavouriteChannel* chan = g_Options.GetFavouriteChannelList().FindChannel(m_sChannel);
			if(chan)
			{
				chan->AutoJoin = !chan->AutoJoin;
			}
			else
			{
				g_Options.GetFavouriteChannelList().AddChannel(m_sChannel, _T(""), true);
			}
		}
		break;
		default:
			return false;
	}
	return true;
}

void ChannelWindow::DoMenu(POINT pt)
{
	_TRACE("ChannelWindow(0x%08X)::DoMenu({%i, %i})", this, pt.x, pt.y);

	CheckMenuItem(m_hMenu, ID_CHANNEL_SHOWUSERLIST, MF_BYCOMMAND | (m_bShowNickList ? MF_CHECKED : MF_UNCHECKED));
	
	FavouriteChannel* chan = g_Options.GetFavouriteChannelList().FindChannel(m_sChannel);
	CheckMenuItem(m_hMenu, ID_CHANNEL_FAVOURITECHANNEL, MF_BYCOMMAND | (chan ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(m_hMenu, ID_CHANNEL_AUTOJOINONCONNECT, MF_BYCOMMAND | (chan && chan->AutoJoin ? MF_CHECKED : MF_UNCHECKED));

	DisplayWindow::DoMenu(pt);
}

/////////////////////////////////////////////////////////////////////////////
//	Utilities
/////////////////////////////////////////////////////////////////////////////

tstring ChannelWindow::GetUserModes()
{
	tstring symbols;
	m_pMainWindow->GetSession()->GetChannelUserModes(NULL, &symbols);
	return symbols;
}

bool ChannelWindow::HasUser(const tstring& sUser)
{
	int index = GetNickListIndex(sUser);
	return index >= 0;
}

bool ListBoxGetString(HWND hList, unsigned index, tstring& str)
{
	int len = SendMessage(hList, LB_GETTEXTLEN, index, 0);
	_ASSERTE(len != LB_ERR);

	if(len != LB_ERR)
	{
		TCHAR* buf = new TCHAR[len + 1];
		SendMessage(hList, LB_GETTEXT, index, (LPARAM)buf);
		str = buf;
		delete[] buf;

		return true;
	}
	return false;
}

tstring ChannelWindow::GetNickListEntry(int index)
{
	tstring sNick;
	if(ListBoxGetString(m_hNickList, index, sNick))
	{
		return StripNick(sNick, GetUserModes());
	}
	return NULL;
}

int ChannelWindow::GetNickListIndex(const tstring& sUser)
{
	int nNicks = SendMessage(m_hNickList, LB_GETCOUNT, 0, 0);
	
	tstring sTest;
	sTest.reserve(POCKETIRC_MAX_NICK_LEN + 1);
	for(int i = 0; i < nNicks; ++i)
	{
		tstring sNick;
		if(ListBoxGetString(m_hNickList, i, sTest))
		{
			if(Compare(sUser, StripNick(sTest, GetUserModes()), false))
			{
				return i;
			}
		}
	}

	return LB_ERR;
}

void ChannelWindow::DoUserOnChannelMenu(WORD x, WORD y, const tstring& sUser)
{
	_TRACE("ChannelWindow(0x%08X)::DoUserOnChannelMenu(%i, %i, \"%s\")", this, x, y, sUser.c_str());

	HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CHANNELUSERMENU));
	_ASSERTE(hMenu != NULL);

	UINT iCmd = TrackPopupMenuEx(GetSubMenu(hMenu, 0), TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, 
		x, y, m_hwnd, NULL);

	tstring sInput = m_pMainWindow->GetInput();

	switch(iCmd)
	{
		case ID_CHANNELUSER_OP:
		{
			tstring sModeString = _T("+o ");
			sModeString += sUser;
			m_pMainWindow->GetSession()->Mode(m_sChannel, sModeString);
		}
		break;
		case ID_CHANNELUSER_DEOP:
		{
			tstring sModeString = _T("-o ");
			sModeString += sUser;
			m_pMainWindow->GetSession()->Mode(m_sChannel, sModeString);
		}
		break;
		case ID_CHANNELUSER_VOICE:
		{
			tstring sModeString = _T("+v ");
			sModeString += sUser;
			m_pMainWindow->GetSession()->Mode(m_sChannel, sModeString);
		}
		break;
		case ID_CHANNELUSER_DEVOICE:
		{
			tstring sModeString = _T("-v ");
			sModeString += sUser;
			m_pMainWindow->GetSession()->Mode(m_sChannel, sModeString);
		}
		break;
		case ID_CHANNELUSER_KICK:
			m_pMainWindow->GetSession()->Kick(m_sChannel, sUser, sInput);
		break;
		case ID_CHANNELUSER_BAN:
		{
			tstring sModeString = _T("+b ");
			sModeString += sUser;
			sModeString += _T("!*@*");
			m_pMainWindow->GetSession()->Mode(m_sChannel, sModeString);
		}
		break;
		case ID_CHANNELUSER_KICKBAN:
		{
			tstring sModeString = _T("+b ");
			sModeString += sUser;
			sModeString += _T("!*@*");
			m_pMainWindow->GetSession()->Mode(m_sChannel, sModeString);
			m_pMainWindow->GetSession()->Kick(m_sChannel, sUser, sInput);
		}
		break;
		case ID_CHANNELUSER_QUERY:
			m_pMainWindow->OpenQuery(sUser);
		break;
		case ID_CHANNELUSER_WHOIS:
			m_pMainWindow->GetSession()->Whois(sUser);
		break;
		case ID_CHANNELUSER_DCCSEND:
			m_pMainWindow->GetDCCHandler()->Send(sUser);
		break;
		case ID_CHANNELUSER_DCCCHAT:
			m_pMainWindow->GetDCCHandler()->Chat(sUser);
		break;
	}

	DestroyMenu(hMenu);
}
