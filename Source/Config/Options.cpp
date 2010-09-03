#include "PocketIRC.h"
#include "Options.h"

#include "Core\NetworkEvent.h"

#include "RegKey.h"
#include "ImageFile.h"

Options g_Options;

/////////////////////////////////////////////////////////////////////////////
//	Formatting
/////////////////////////////////////////////////////////////////////////////

static struct _POCKETIRC_FORMAT {
	int id;
	bool in;
	bool enable;
	LPCTSTR def;
	LPCTSTR name;
} POCKETIRC_FORMAT[] = {
	{IRC_RPL_MOTD, true, false, _T("$1"), _T("MOTD Reply")},
	{IRC_RPL_TOPIC, true, true, _T("\x03") _T("3* $1 topic: $2"), _T("Topic Reply")},
	{IRC_RPL_LIST, true, false, _T("$1 [$2] $3"), _T("List Reply")},

	{IRC_ERR_CANNOTSENDTOCHAN, true, true, _T("\x03") _T("12* Cannot send to $1"), _T("Err Can't Send")},
	{IRC_ERR_NOTONCHANNEL, true, true, _T("\x03") _T("12* You're not on $1"), _T("Err Not On Channel")},
	{IRC_ERR_KEYSET, true, true, _T("\x03") _T("12* $1 already has a key"), _T("Err Key Set")},
	{IRC_ERR_CHANNELISFULL, true, true,_T("\x03")  _T("12* $1 is full"), _T("Err Channel Full")},
	{IRC_ERR_INVITEONLYCHAN, true, true, _T("\x03") _T("12* $1 is invite only"), _T("Err Invite Only")},
	{IRC_ERR_BANNEDFROMCHAN, true, true, _T("\x03") _T("12* You are banned from $1"), _T("Err Banned")},
	{IRC_ERR_BADCHANNELKEY, true, true, _T("\x03") _T("12* Wrong key for $1"), _T("Err Bad Key")},
	{IRC_ERR_BADCHANMASK, true, true, _T("\x03") _T("12* Bad channel mask $1"), _T("Err Bad Mask")}, //what does this mean?
	{IRC_ERR_NOCHANMODES, true, true, _T("\x03") _T("12* $1 doesn't support modes"), _T("Err No Chan Modes")},
	{IRC_ERR_BANLISTFULL, true, true, _T("\x03") _T("12* $1 ban list is full"), _T("Err List Full")},
	{IRC_ERR_CHANOPRIVSNEEDED, true, true, _T("\x03") _T("12* You're not an operator on $1"), _T("Err Not Chan-Op")},
	{IRC_ERR_USERNOTINCHANNEL, true, true, _T("\x03") _T("12* $1 is not on $2"), _T("Err User Not In Channel")},
	{IRC_ERR_USERONCHANNEL, true, true, _T("\x03") _T("12* $1 is already on $2"), _T("Err User On Channel")},

	{IRC_CMD_PASS, false, true, _T("PASS *****"), _T("Pass Out")},
	{IRC_CMD_USER, false, true, _T("USER $0 $3"), _T("User Out")},

	{IRC_CMD_TOPIC, true, true, _T("\x03") _T("3* $n sets topic: $1"), _T("Topic In")},
	{IRC_CMD_TOPIC, false, false, _T("$c $*"), _T("Topic Out")},
	{IRC_CMD_NICK, true, true, _T("\x03") _T("3* $n is now $0"), _T("Nick In")},
	{IRC_CMD_NICK, false, true, _T("$c $*"), _T("Nick Out")},
	{IRC_CMD_JOIN, true, true, _T("\x03") _T("3* $n joins $0"), _T("Join In")},
	{IRC_CMD_JOIN, false, false, _T("$c $*"), _T("Join Out")},
	{IRC_CMD_PART, true, true, _T("\x03") _T("3* $n leaves $0"), _T("Part In")},
	{IRC_CMD_PART, false, false, _T("$c $*"), _T("Part Out")},
	{IRC_CMD_QUIT, true, true, _T("\x03") _T("2* $n quits ($0)"), _T("Quit In")},
	{IRC_CMD_QUIT, false, false, _T("$c $*"), _T("Quit Out")},
	{IRC_CMD_KICK, true, true, _T("\x03") _T("3* $n kicks $1 from $0 ($2)"), _T("Kick In")},
	{IRC_CMD_KICK, false, false, _T("$c $*"), _T("Kick Out")},
	{IRC_CMD_MODE, true, true, _T("\x03") _T("3* $n sets mode: $1-"), _T("Mode In")},
	{IRC_CMD_MODE, false, false, _T("$c $*"), _T("Mode Out")},
	{IRC_CMD_PRIVMSG, true, true, _T("<$n> $1"), _T("PrivMsg In")},
	{IRC_CMD_PRIVMSG, false, true, _T("<$u> $1"), _T("PrivMsg Out")},
	{IRC_CMD_NOTICE, true, true, _T("\x03") _T("5-$n- $1"), _T("Notice In")},
	{IRC_CMD_NOTICE, false, true, _T("\x03") _T("5+$0+ $1"), _T("Notice Out")},
	{IRC_CMD_INVITE, true, true, _T("\x03") _T("3* $n invites you to $1"), _T("Invite In")},
	{IRC_CMD_INVITE, false, true, _T("\x03") _T("3* $0 invited to $1"), _T("Invite Out")},
	{IRC_CMD_PING, true, false, _T("\x03") _T("3$c $*"), _T("Ping In")},
	{IRC_CMD_PING, false, false, _T("$c $*"), _T("Ping Out")},
	{IRC_CMD_PONG, true, false, _T("\x03") _T("3$c $*"), _T("Pong In")},
	{IRC_CMD_PONG, false, false, _T("$c $*"), _T("Pong Out")},


	{IRC_CTCP_ACTION, true, true, _T("\x03") _T("6* $n $1"), _T("Action In")},
	{IRC_CTCP_ACTION, false, true, _T("\x03") _T("6* $u $1"), _T("Action Out")},
	{IRC_CTCP_RPL_PING, true, true, _T("\x03") _T("4[$n] $c reply $2- seconds"), _T("CTCP Ping Reply In")},

	{SYS_EVENT_TRYCONNECT, false, true, _T("\x03") _T("12Trying $0 ..."), _T("Try Connect")},
	{SYS_EVENT_WAITCONNECT, false, true, _T("\x03") _T("12Waiting for $0 ..."), _T("Wait For Connect")},
	{SYS_EVENT_CONNECTED, false, true, _T("\x03") _T("12Connected to $0"), _T("Connected")},
	{SYS_EVENT_CONNECTFAILED, false, true, _T("\x03") _T("12Connect failed: $0"), _T("Connect Failed")},
	{SYS_EVENT_CLOSE, false, true, _T("\x03") _T("12Disconnected"), _T("Disconnected")},
	{SYS_EVENT_STATUS, false, true, _T("\x03") _T("12Status: $*"), _T("Status")},
	{SYS_EVENT_IDENT, false, true, _T("\x03") _T("12IDENT request: $*"), _T("Ident Request")},
	{SYS_EVENT_ERROR, false, true, _T("\x03") _T("12Error: $*"), _T("Error")},

	{IRC_CMD_UNKNOWN, true, true, _T("\x03") _T("3$c $*"), _T("Other In")},
	{IRC_CMD_UNKNOWN, false, true, _T("$c $*"), _T("Other Out")},
	{IRC_RPL_UNKNOWN, true, true, _T("$1-"), _T("Other Reply")},
	{IRC_CTCP_UNKNOWN, true, true, _T("\x03") _T("4[$n] $c $1-"), _T("Other CTCP In")},
	{IRC_CTCP_UNKNOWN, false, true, _T("\x03") _T("4$c [$0] $1-"), _T("Other CTCP Out")},
	{IRC_CTCP_RPL_UNKNOWN, true, true, _T("\x03") _T("4[$n] $c reply $1-"), _T("Other CTCP Reply In")},
	{IRC_CTCP_RPL_UNKNOWN, false, false, _T("\x03") _T("4$c reply [$0] $1-"), _T("Other CTCP Reply Out")},
	{SYS_EVENT_UNKNOWN, false, true, _T("\x03") _T("12[$c] $*"), _T("Other Status")},
};

static HFONT g_hfDisplay = NULL;
static HFONT g_hfControl = NULL;
static HBITMAP g_hbmBack = NULL;
static HBRUSH g_hbrBack = NULL;

static tstring g_sLocalAddress;
static tstring g_sDetectedAddress;


/////////////////////////////////////////////////////////////////////////////
//	Static Utility Methods
/////////////////////////////////////////////////////////////////////////////
const tstring& Options::GetEventFormat(int idEvent, bool bIncoming)
{
	Format* pFormat = g_Options.GetFormatList().GetFormat(idEvent, bIncoming);
	_ASSERTE(pFormat != NULL);

	return pFormat->GetFormat();
}

const tstring Options::GetDefaultFormat(int idEvent, bool bIncoming)
{
	for(int i = 0; i < sizeof(POCKETIRC_FORMAT)/sizeof(POCKETIRC_FORMAT[0]); ++i)
	{
		if(POCKETIRC_FORMAT[i].id == idEvent && POCKETIRC_FORMAT[i].in == bIncoming)
		{
			return POCKETIRC_FORMAT[i].def;
		}
	}
	return tstring();
}

bool Options::GetDefaultEnable(int idEvent, bool bIncoming)
{
	for(int i = 0; i < sizeof(POCKETIRC_FORMAT)/sizeof(POCKETIRC_FORMAT[0]); ++i)
	{
		if(POCKETIRC_FORMAT[i].id == idEvent && POCKETIRC_FORMAT[i].in == bIncoming)
		{
			return POCKETIRC_FORMAT[i].enable;
		}
	}
	return true;
}

HFONT Options::GetDisplayFont()
{
	return g_hfDisplay;	
}

HFONT Options::GetControlFont()
{
	return g_hfControl;	
}

HBITMAP Options::GetDisplayBackground()
{
	return g_hbmBack;
}

HBRUSH Options::GetBackColorBrush()
{
	return g_hbrBack;
}

void Options::UpdateResourceCache()
{
	// Display Background
	if(g_hbmBack)
	{
		DeleteObject(g_hbmBack);
		g_hbmBack = NULL;
	}

	if(g_Options.GetBackImage().size() > 0)
	{
		g_hbmBack = ImageFileLoad(g_Options.GetBackImage().c_str());
	}

	if(g_hbrBack)
	{
		DeleteObject(g_hbrBack);
	}
	g_hbrBack = CreateSolidBrush(g_Options.GetBackColor());

	// Display Font
	if(g_hfDisplay)
	{
		DeleteObject(g_hfDisplay);
		g_hfDisplay = NULL;
	}

	LOGFONT lf;
	ZeroMemory(&lf, sizeof(lf));
	lf.lfQuality = DEFAULT_QUALITY;

	_tcsncpy(lf.lfFaceName, g_Options.GetFontName().c_str(), sizeof(lf.lfFaceName)/sizeof(TCHAR));
	lf.lfFaceName[sizeof(lf.lfFaceName)/sizeof(TCHAR) - 1] = '\0';

	int logpix = GetDeviceCaps(GetDC(NULL), LOGPIXELSY);
	lf.lfHeight = -(long)((((double)g_Options.GetFontSize() * (double)logpix) / (double)72) + 0.5);

	g_hfDisplay = CreateFontIndirect(&lf);

	// Control Font
	if(g_hfControl == NULL)
	{
		ZeroMemory(&lf, sizeof(lf));
		lf.lfQuality = DEFAULT_QUALITY;

		_tcscpy(lf.lfFaceName, _T("Tahoma"));

		lf.lfHeight = -(long)((((double)8 * (double)logpix) / (double)72) + 0.5);

		g_hfControl = CreateFontIndirect(&lf);
	}
}

bool Options::IsRegistered()
{
//#ifndef DEBUG
//	DWORD dwCode = 0x0BADBABE;
//	DWORD dwReg = 0;
//	dwReg = _tcstoul(g_Options.GetRegCode().c_str(), NULL, 16);
//
//	return (~dwReg == dwCode) || PresButan();
//#else
	// Always registered as of 2010-06-12 - Brook 
	return true;
//#endif
}

bool Options::PresButan()
{
	return (_tcscmp(g_Options.GetRegCode().c_str(), _T("pres butan")) == 0);
}

void Options::SetLocalAddress(const tstring& sAddr)
{
	g_sLocalAddress = sAddr;
}

void Options::SetDetectedAddress(const tstring& sAddr)
{
	g_sDetectedAddress = sAddr;
}

tstring Options::GetLocalAddress()
{
	return g_sLocalAddress;
}

tstring Options::GetDetectedAddress()
{
	return g_sDetectedAddress;
}

tstring Options::GetAddress()
{
	if(g_Options.GetLocalAddressMethod() == 0)
	{
		return g_sLocalAddress;
	}
	else if(g_Options.GetLocalAddressMethod() == 1)
	{
		return g_sDetectedAddress;
	}
	else if(g_Options.GetLocalAddressMethod() == 2)
	{
		return g_Options.GetCustomAddress();
	}
	_ASSERTE(FALSE);
	return 0;
}

void Options::SetDefaultHost(const tstring& sHost, USHORT uPort, const tstring& sPass)
{
	// Add to server list
	HostList& lstHosts = GetHostList();

	Host* pHost = NULL;
	HRESULT hr = lstHosts.FindHost(sHost, &pHost);
	_ASSERTE(SUCCEEDED(hr));

	if(hr == S_OK)
	{
		pHost->SetPort(uPort);
	}
	else
	{
		hr = lstHosts.AddHost(sHost, uPort, &pHost);
		_ASSERTE(SUCCEEDED(hr));
		_ASSERTE(pHost != NULL);
	}

	pHost->SetPass(sPass);

	lstHosts.SetDefault(pHost);
}

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

Options::Options()
{
	_TRACE("Options(0x%08X)::Options()", this);

	m_crHighlight[0] = RGB(0, 0, 0);
	m_crHighlight[1] = RGB(0, 0, 255);
	m_crHighlight[2] = RGB(255, 0, 0);
	m_crHighlight[3] = RGB(0, 255, 0);
}

Options::Options(Options& from)
{
	_TRACE("Options(0x%08X)::Options(0x%08X)", this, &from);

	m_crHighlight[0] = RGB(0, 0, 0);
	m_crHighlight[1] = RGB(0, 0, 255);
	m_crHighlight[2] = RGB(255, 0, 0);
	m_crHighlight[3] = RGB(0, 255, 0);

	*this = from;
}

Options::~Options()
{
	_TRACE("Options(0x%08X)::~Options()", this);

}

/////////////////////////////////////////////////////////////////////////////
//	Assignment Operator
/////////////////////////////////////////////////////////////////////////////

const Options& Options::operator=(Options& from)
{	
	_TRACE("Options(0x%08X)::operator=(0x%08X)", this, &from);

	// Server
	SetNick(from.GetNick());
	SetRealName(from.GetRealName());
	SetQuitMsg(from.GetQuitMsg());
	SetUTF8(from.GetUTF8());
	m_lstHosts = from.GetHostList();

	// Channels
	m_favChannels = from.GetFavouriteChannelList();
	SetChanListUserMin(from.GetChanListUserMin());

	// Display
	SetFullScreen(from.GetFullScreen());
	SetBackAlignTop(from.GetBackAlignTop());

	m_lstFormats = from.GetFormatList();
	for(int i = 0; i < sizeof(m_crHighlight)/sizeof(m_crHighlight[0]); ++i)
	{
		SetHighlight(i, from.GetHighlight(i));
	}
	SetBackImage(from.GetBackImage());

	SetStripAll(from.GetStripAll());
	SetStripIncoming(from.GetStripIncoming());
	SetShowTimestamp(from.GetShowTimestamp());

	// Ident
	SetIdentEnable(from.GetIdentEnable());
	SetIdentUser(from.GetIdentUser());
	SetIdentPort(from.GetIdentPort());

	// Format
	SetTextColor(from.GetTextColor());
	SetBackColor(from.GetBackColor());
	SetFontName(from.GetFontName());
	SetFontSize(from.GetFontSize());
	
	// DCC
	SetLocalAddressMethod(from.GetLocalAddressMethod());
	SetCustomAddress(from.GetCustomAddress());
	SetDCCStartPort(from.GetDCCStartPort());
	SetDCCEndPort(from.GetDCCEndPort());
	SetIgnoreChat(from.GetIgnoreChat());
	SetIgnoreSend(from.GetIgnoreSend());

	// Aboot
	SetRegCode(from.GetRegCode());

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////
void Options::SetRealName(const tstring& sRealName) 
{
	if(IsRegistered())
	{
		m_sRealName = sRealName; 
	}
}

void Options::SetQuitMsg(const tstring& sQuitMsg) 
{
	if(IsRegistered())
	{
		m_sQuitMsg = sQuitMsg; 
	}
}

const tstring Options::GetRealName() 
{
	if(IsRegistered())
	{
		return m_sRealName; 
	}
	else
	{
		return POCKETIRC_UNREGISTERED_REALNAME;
	}
}

const tstring Options::GetQuitMsg() 
{
	if(IsRegistered())
	{
		return m_sQuitMsg; 
	}
	else
	{
		return POCKETIRC_UNREGISTERED_QUIT_MESSAGE;
	}
}

HRESULT Options::Save(HKEY hkRoot, const tstring& sKeyName)
{
	_TRACE("Options(0x%08X)::Save(0x%08X, \"%s\")", this, hkRoot, sKeyName.c_str());
	_ASSERTE(hkRoot != NULL);

	RegKey key(hkRoot, sKeyName);

	key.SetString(_T("RealName"), m_sRealName);
	key.SetString(_T("NickName"), m_sNick);
	key.SetString(_T("QuitMsg"), m_sQuitMsg);
	key.SetBool(_T("UTF8"), m_bUTF8);

	// Host List
	RegKey keyHosts(key, _T("Host"));
	keyHosts.Clear();

	Host* pDefHost = m_lstHosts.GetDefault();
	if(pDefHost)
	{
		keyHosts.SetString(_T(""), pDefHost->GetAddress());
	}

	for(UINT i = 0; i < m_lstHosts.Count(); ++i)
	{
		Host* pHost = NULL;
		m_lstHosts.Item(i, &pHost);
		_ASSERTE(pHost != NULL);

		RegKey keyHost(keyHosts, pHost->GetAddress());

		keyHost.SetDWORD(_T("Port"), pHost->GetPort());
		keyHost.SetString(_T("Pass"), pHost->GetPass());
	}
	keyHosts.Close();

	key.SetBool(_T("FullScreen"), m_bFullScreen);
	key.SetBool(_T("BackAlignTop"), m_bBackAlignTop);
	key.SetString(_T("BackImage"), m_sBackImage);
	key.SetBool(_T("StripIncoming"), m_bStripIncoming);
	key.SetBool(_T("StripAll"), m_bStripAll);
	key.SetBool(_T("ShowTimestamp"), m_bShowTimestamp);

	// Channels List
	key.SetDWORD(_T("ChannelListMinUsers"), m_nChanListMinUsers);

	RegKey keyChannels(key, _T("Channels"));
	keyChannels.Clear();

	for(UINT i = 0; i < m_favChannels.Count(); ++i)
	{
		TCHAR buf[20] = _T("");
		_sntprintf(buf, sizeof(buf)/sizeof(buf[0]), _T("%u"), i);

		RegKey keyChan(keyChannels, buf);

		FavouriteChannel* chan = m_favChannels.GetChannel(i);

		keyChan.SetString(_T("Name"), chan->Name);
		keyChan.SetString(_T("Key"), chan->Key);
		keyChan.SetBool(_T("AutoJoinOnConnect"), chan->AutoJoin);
	}
	keyChannels.Close();

	// Event Format List
	RegKey keyFormats(key, _T("Format"));
	for(int j = 0; j < sizeof(POCKETIRC_FORMAT)/sizeof(POCKETIRC_FORMAT[0]); ++j)
	{
		Format* pFormat = m_lstFormats.GetFormat(POCKETIRC_FORMAT[j].id, POCKETIRC_FORMAT[j].in);
		_ASSERTE(pFormat != NULL);

		RegKey keyFormat(keyFormats, POCKETIRC_FORMAT[j].name);

		keyFormat.SetString(_T(""), pFormat->GetFormat());
		keyFormat.SetBool(_T("Display"), pFormat->IsEnabled());
	}
	keyFormats.Close();

	// Ident
	key.SetBool(_T("IdentEnable"), m_bIdentEnable);
	key.SetString(_T("IdentUser"), m_sIdentUser);
	key.SetDWORD(_T("IdentPort"), m_uIdentPort);

	// Format
	key.SetDWORD(_T("BackColor"), m_crBackColor);
	key.SetDWORD(_T("TextColor"), m_crTextColor);
	key.SetString(_T("FontName"), m_sFontName);
	key.SetDWORD(_T("FontSize"), m_dwFontSize);

	// DCC
	key.SetDWORD(_T("LocalAddressMethod"), m_dwLocalAddressMethod);
	key.SetString(_T("CustomAddress"), m_sCustomAddress);
	key.SetDWORD(_T("DCCStartPort"), m_uDCCStartPort);
	key.SetDWORD(_T("DCCEndPort"), m_uDCCEndPort);
	key.SetBool(_T("IgnoreChat"), m_bIgnoreChat);
	key.SetBool(_T("IgnoreSend"), m_bIgnoreSend);

	// About
	key.SetString(_T("RegCode"), m_sRegCode);

	return S_OK;
}

HRESULT Options::Load(HKEY hkRoot, const tstring& sKeyName)
{
	_TRACE("Options(0x%08X)::Load(0x%08X, \"%s\")", this, hkRoot, sKeyName.c_str());
	_ASSERTE(hkRoot != NULL);

	RegKey key(hkRoot, sKeyName);

	m_sRealName = key.GetString(_T("RealName"), POCKETIRC_DEF_REALNAME);
	m_sNick = key.GetString(_T("NickName"), POCKETIRC_DEF_NICK);
	m_sQuitMsg = key.GetString(_T("QuitMsg"), POCKETIRC_DEF_QUIT_MESSAGE);
	m_bUTF8 = key.GetBool(_T("UTF8"), true);

	// Host List
	RegKey keyHosts(key, _T("Host"));
	tstring sDefHost = keyHosts.GetString(_T(""), _T(""));
	
	m_lstHosts.Clear();
	DWORD dwIndex = 0;
	tstring sHost;
	while(keyHosts.EnumKey(dwIndex++, sHost))
	{
		RegKey keyHost(keyHosts, sHost);
		USHORT uPort = (USHORT)keyHost.GetDWORD(_T("Port"), 6667);
		tstring sPass = keyHost.GetString(_T("Pass"), _T(""));

		Host* pHost = NULL;
		m_lstHosts.AddHost(sHost, uPort, &pHost);
		_ASSERTE(pHost != NULL);

		pHost->SetPass(sPass);

		if(_tcsicmp(sDefHost.c_str(), sHost.c_str()) == 0)
		{
			m_lstHosts.SetDefault(pHost);
		}

		keyHost.Close();
	}
	keyHosts.Close();

	m_bFullScreen = key.GetBool(_T("FullScreen"), false);
	m_bBackAlignTop = key.GetBool(_T("BackAlignTop"), true);
	m_sBackImage = key.GetString(_T("BackImage"), _T(""));

	m_bStripAll = key.GetBool(_T("StripAll"), false);
	m_bStripIncoming = key.GetBool(_T("StripIncoming"), false);
	m_bShowTimestamp = key.GetBool(_T("ShowTimestamp"), false);

	// Channels List

	m_nChanListMinUsers = (UINT)key.GetDWORD(_T("ChannelListMinUsers"), 3);

	m_favChannels.Clear();

	RegKey keyChannels(key, _T("Channels"));
	tstring sChannel;
	DWORD dwChan = 0;
	while(keyChannels.EnumKey(dwChan++, sChannel))
	{
		RegKey keyChan(keyChannels, sChannel);

		tstring sName = keyChan.GetString(_T("Name"), _T(""));
		tstring sKey = keyChan.GetString(_T("Key"), _T(""));
		bool autoJoin = keyChan.GetBool(_T("AutoJoinOnConnect"), false);

		if(sName != tstring(_T("")))
			m_favChannels.AddChannel(sName, sKey, autoJoin);
	}

	// Event Format List
	RegKey keyFormats(key, _T("Format"));
	for(int i = 0; i < sizeof(POCKETIRC_FORMAT)/sizeof(POCKETIRC_FORMAT[0]); ++i)
	{
		Format* pFormat = new Format();
	
		pFormat->SetEventName(POCKETIRC_FORMAT[i].name);
		pFormat->SetEventID(POCKETIRC_FORMAT[i].id);
		pFormat->SetIncoming(POCKETIRC_FORMAT[i].in);

		RegKey keyFormat(keyFormats, POCKETIRC_FORMAT[i].name);

		pFormat->SetFormat(keyFormat.GetString(_T(""), POCKETIRC_FORMAT[i].def));
		pFormat->SetEnabled(keyFormat.GetBool(_T("Display"), POCKETIRC_FORMAT[i].enable));

		keyFormat.Close();

		m_lstFormats.AddFormat(pFormat);
	}
	keyFormats.Close();

	// Ident
	m_bIdentEnable = key.GetBool(_T("IdentEnable"), false);
	m_sIdentUser = key.GetString(_T("IdentUser"), POCKETIRC_DEF_IDENT);
	m_uIdentPort = (USHORT)key.GetDWORD(_T("IdentPort"), 113);

	// Format
	m_crBackColor = key.GetDWORD(_T("BackColor"), RGB(255, 255, 255));
	m_crTextColor = key.GetDWORD(_T("TextColor"), RGB(0, 0, 0));
	m_sFontName = key.GetString(_T("FontName"), _T("Tahoma"));
	m_dwFontSize = key.GetDWORD(_T("FontSize"), 8);

	// DCC
	m_dwLocalAddressMethod = key.GetDWORD(_T("LocalAddressMethod"), 0);
	m_sCustomAddress = key.GetString(_T("CustomAddress"), _T(""));
	m_uDCCStartPort = (USHORT)key.GetDWORD(_T("DCCStartPort"), 5000);
	m_uDCCEndPort = (USHORT)key.GetDWORD(_T("DCCEndPort"), 5100);
	m_bIgnoreChat = key.GetBool(_T("IgnoreChat"), false);
	m_bIgnoreSend = key.GetBool(_T("IgnoreSend"), false);

	// About
	m_sRegCode = key.GetString(_T("RegCode"), _T(""));

	return S_OK;
}
