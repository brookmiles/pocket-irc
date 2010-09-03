#ifndef _OPTIONS_H_INCLUDED_
#define _OPTIONS_H_INCLUDED_

#include "HostList.h"
#include "FormatList.h"
#include "Channels.h"

class Options
{
public:
// Utility Accessor Methods
	static const tstring& GetEventFormat(int idEvent, bool bIncoming);
	static const tstring GetDefaultFormat(int idEvent, bool bIncoming);
	static bool GetDefaultEnable(int idEvent, bool bIncoming);
	static HBITMAP GetDisplayBackground();
	static HFONT GetDisplayFont();
	static HFONT GetControlFont();
	static HBRUSH GetBackColorBrush();
	static void UpdateResourceCache();
	static bool IsRegistered();
	static bool PresButan();

	static void SetLocalAddress(const tstring& sAddr);
	static void SetDetectedAddress(const tstring& sAddr);
	static tstring GetLocalAddress();
	static tstring GetDetectedAddress();
	static tstring GetAddress();

	Options();
	Options(Options& from);
	~Options();

	const Options& operator=(Options& from);

	HRESULT Save(HKEY hkRoot, const tstring& sKeyName);
	HRESULT Load(HKEY hkRoot, const tstring& sKeyName);

// Server
	void SetNick(const tstring& sNick) { m_sNick = sNick; }
	const tstring& GetNick() { return m_sNick; }

	void SetRealName(const tstring& sRealName);
	const tstring GetRealName();

	void SetQuitMsg(const tstring& sQuitMsg);
	const tstring GetQuitMsg();

	HostList& GetHostList() { return m_lstHosts; }
	void SetDefaultHost(const tstring& sHost, USHORT uPort, const tstring& sPass);

	void SetUTF8(bool bUTF8) { m_bUTF8 = bUTF8; }
	bool GetUTF8() { return m_bUTF8; }

// Channel Favourites

	FavouriteChannelList& GetFavouriteChannelList() { return m_favChannels; }

	void SetChanListUserMin(UINT nChanListMinUsers) { m_nChanListMinUsers = nChanListMinUsers; }
	UINT GetChanListUserMin() { return m_nChanListMinUsers; }

// Display
	void SetFullScreen(bool bFullScreen) { m_bFullScreen = bFullScreen; }
	bool GetFullScreen() { return m_bFullScreen; }

	FormatList& GetFormatList() { return m_lstFormats; }

	void SetHighlight(UINT iHighlight, COLORREF cr) { m_crHighlight[iHighlight] = cr; }
	COLORREF GetHighlight(UINT iHighlight) { return m_crHighlight[iHighlight]; }

	void SetBackImage(const tstring& sBackImage) { m_sBackImage = sBackImage; }
	const tstring& GetBackImage() { return m_sBackImage; }

	void SetBackAlignTop(bool bBackAlignTop) { m_bBackAlignTop = bBackAlignTop; }
	bool GetBackAlignTop() { return m_bBackAlignTop; }

	void SetStripIncoming(bool bStripIncoming) { m_bStripIncoming = bStripIncoming; }
	bool GetStripIncoming() { return m_bStripIncoming; }

	void SetStripAll(bool bStripAll) { m_bStripAll = bStripAll; }
	bool GetStripAll() { return m_bStripAll; }

	void SetShowTimestamp(bool bShowTimestamp) { m_bShowTimestamp = bShowTimestamp; }
	bool GetShowTimestamp() { return m_bShowTimestamp; }

// Ident
	void SetIdentEnable(bool bIdentEnable) { m_bIdentEnable = bIdentEnable; }
	bool GetIdentEnable() { return m_bIdentEnable; }

	void SetIdentUser(const tstring& sIdentUser) { m_sIdentUser = sIdentUser; }
	const tstring& GetIdentUser() { return m_sIdentUser; }

	void SetIdentPort(USHORT uIdentPort) { m_uIdentPort = uIdentPort; }
	USHORT GetIdentPort() { return m_uIdentPort; }

// Format
	void SetBackColor(COLORREF cr) { m_crBackColor = cr; }
	COLORREF GetBackColor() { return m_crBackColor; }

	void SetTextColor(COLORREF cr) { m_crTextColor = cr; }
	COLORREF GetTextColor() { return m_crTextColor; }

	void SetFontName(const tstring& sFontName) { m_sFontName = sFontName; }
	const tstring& GetFontName() { return m_sFontName; }

	void SetFontSize(DWORD dw) { m_dwFontSize = dw; }
	DWORD GetFontSize() { return m_dwFontSize; }

// DCC
	void SetLocalAddressMethod(DWORD dw) { m_dwLocalAddressMethod = dw; }
	DWORD GetLocalAddressMethod() { return m_dwLocalAddressMethod; }

	void SetCustomAddress(const tstring& sCustomAddress) { m_sCustomAddress = sCustomAddress; }
	const tstring& GetCustomAddress() { return m_sCustomAddress; }

	void SetDCCStartPort(USHORT uDCCStartPort) { m_uDCCStartPort = uDCCStartPort; }
	USHORT GetDCCStartPort() { return m_uDCCStartPort; }
	
	void SetDCCEndPort(USHORT uDCCEndPort) { m_uDCCEndPort = uDCCEndPort; }
	USHORT GetDCCEndPort() { return m_uDCCEndPort; }

	void SetIgnoreChat(bool bIgnoreChat) { m_bIgnoreChat = bIgnoreChat; }
	bool GetIgnoreChat() { return m_bIgnoreChat; }

	void SetIgnoreSend(bool bIgnoreSend) { m_bIgnoreSend = bIgnoreSend; }
	bool GetIgnoreSend() { return m_bIgnoreSend; }

// About

	void SetRegCode(const tstring& sRegCode) { m_sRegCode = sRegCode; }
	const tstring& GetRegCode() { return m_sRegCode; }

protected:
	tstring	m_sNick;
	tstring	m_sRealName;
	HostList m_lstHosts;
	tstring	m_sQuitMsg;
	bool m_bUTF8;

	FavouriteChannelList m_favChannels;
	UINT m_nChanListMinUsers;

	bool		m_bFullScreen;
	bool		m_bBackAlignTop;
	COLORREF	m_crHighlight[4];
	FormatList	m_lstFormats;
	tstring		m_sBackImage;
	bool		m_bStripAll;
	bool		m_bStripIncoming;
	bool		m_bShowTimestamp;

	bool	m_bIdentEnable;
	tstring	m_sIdentUser;
	USHORT	m_uIdentPort;

	COLORREF m_crBackColor;
	COLORREF m_crTextColor;
	tstring	m_sFontName;
	DWORD	m_dwFontSize;

	DWORD m_dwLocalAddressMethod;
	tstring m_sCustomAddress;
	USHORT m_uDCCStartPort;
	USHORT m_uDCCEndPort;
	bool m_bIgnoreChat;
	bool m_bIgnoreSend;

	tstring m_sRegCode;
};

extern Options g_Options;

#endif//_OPTIONS_H_INCLUDED_