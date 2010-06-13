
BUGS
o Test svn commit hook
o Can't Send/Receive files from storage card
o DCC not recognised in some formats, notice type maybe?
o Replace "[ ... ]" with "" in channel list
o Unlamify "Trying..." and other status messages

TODO

o Multiple codepage support
o Logging

o Per network/server? channel lists
o Take channel list out of session object
o Auto Reconnect on Disconnection
o Secondary/generated nick

o Resizeable user list
o Network server lists
o DNS lookup on server provided host info
o Make /list window headers sortable
o Show user list by default option
o Hold to scroll right/left through tabs
o Highlight tab arrow with highest notification colour

o Notify list
o Ignore list
o Channel mode window
o Top of window info bar on disconnect/kick/DCC
o Key combinations for navigating the interface, tabs
o Scroll dialogs into view when tabbing to controls off screen
o Tweak scroll behavior when SIP raised/lowered, try to maintain view
o Auto Rejoin on Kick
o DCC Progress meter
o Open/Run/Play files from DCC window

? Simply socket sink to a single callback with messages
? Auto-connect on run
? Single thread socket handling
? DLL Plugins
? Event scripts
? Configurable command menus
? Tray Notifications
? Audio/Vibration notifications?
? Have TabStrip autosize itself instead of MainWindow doing so
? Double click nick to query
? General perform on connect script
? Per network join on connect and perform

x Server capability parsing
x Doesn't properly parse user mode flags like % and &, general problem of any non-EFNet server flag compatibility
x Channel list breaks on non-breaking spaces in nicknames !#%#@  (irc.visp.ru, #sarovnet, "Dr Geegle")
x User changes nick with query open can no longer talk to that user
x Make DCC List window landscape/square friendly
? drano - can't add favourite to channel (with key?)
x psyBNC compat, has usernames starting with -, ( and )
? psyBNC compat, joining channel with many users causes an error (no other details known)
x Remove duplicates when adding stuff to input combo box
x Enable green button at all times since you can always use it :/
x /server and /connect behaviour when already connected
x Channel List doesn't parse some channels correctly
x Remember channel list (per session)
x Landscape friendly Options dlg
x Landscape friendly Channels dlg
x Add List tab to Channels window and ability to join by typing in a new one
x Timestamps
x Allow querying nicks with ' in them?
x Support /commands
x Better Win32 error code reporting
x Popup menu on channel window too, not just tab
x Perform on connect / join channel / channel favourites
x Nick list menu
x Disable popup menus when disconnected
x DCC chat
x DCC file transfer
x URL hotlinks in display
x Popup menu for nicks and channels in display



XML MENU

<MENU NAME="Channel">
	<COMMAND NAME="Part">
		Part
	</COMMAND>
	<COMMAND NAME="Say">
		Say(InputBar.GetText());
 	</COMMAND>
	<COMMAND NAME="Topic">
		Topic(InputBar.GetText());
 	</COMMAND>
	<SUBMENU NAME="Modes">
		<COMMAND NAME="+s">	
			Mode("+s");
		</COMMAND>
		<COMMAND NAME="+s">	
			Mode("+s");
		</COMMAND>
	</SUBMENU>
</MENU>

<MENU NAME="Query">
	<COMMAND NAME="Close">
		Close
	</COMMAND>
	<COMMAND NAME="Say">
		Say(InputBar.GetText());
 	</COMMAND>
</MENU>



INCOMING EVENT

o Session gets parsed IRCEvent
o Session updates internal state based on
  NICK or MODE changes
  JOIN, PART, KICK events, remote QUITs
o Session optionally passes event to script engine
o If script engine indicates the message should not be processed further, break.
o Session handles PING replies, creates QUERY windows if required
o Event passed to output dispatcher for display on the screen

USER INPUT

o Parse command into IRCCommand object
o Pass IRCCommand to script engine
o If script engine indicates the command should not be processed further, break
o Build in or scripted handler builds an IRCEvent
o IRCEvent is passed to IRCSession
o Session passes Event to OutputDispatcher for display
o IRCSession passed IRCEvent to IRCWriter which formats the event for network transport
o Outgoing message is written to tranport


CHANNEL MODES

	O - give "channel creator" status;
    o - give/take channel operator privilege;
    v - give/take the voice privilege;

    a - toggle the anonymous channel flag;
    i - toggle the invite-only channel flag;
    m - toggle the moderated channel;
    n - toggle the no messages to channel from clients on the
        outside;
    q - toggle the quiet channel flag;
    p - toggle the private channel flag;
    s - toggle the secret channel flag;
    r - toggle the server reop channel flag;
    t - toggle the topic settable by channel operator only flag;

    k - set/remove the channel key (password);
    l - set/remove the user limit to channel;

    b - set/remove ban mask to keep users out;
    e - set/remove an exception mask to override a ban mask;
    I - set/remove an invitation mask to automatically override
        the invite-only flag;

MENU COMMANDS

- Global Commands
	JOIN (input)
	QUIT (input)
	QUERY (input)
	WHOIS (input)
	CTCP (input)
	NICK (input)
	MODE (self) (input)
	RAW (input)
	DCC
		SEND (input)
		XMIT (input)
		CHAT (input)

- Channel
	SAY (channel) (input)
	ACT (channel) (input)
	NOTICE (channel) (input)
	CTCP (channel) (input)
	PART (channel)
	MODE (channel) (*input)
	INVITE (channel) (input)
	User
		VOICE (channel) (input)
		DEVOICE (channel) (input)
		OP (channel) (input)
		DEOP (channel) (input)
		KICK (channel) (input)
		BAN (channel) (input)
		KICK/BAN (channel) (input)

- Channel User
	VOICE (channel) (user)
	DEVOICE (channel) (user)
	OP (channel) (user)
	DEOP (channel) (user)
	KICK (channel) (user) (*input)
	BAN (channel) (user)
	KICK/BAN (channel) (user) (*input)
	User
		MSG (user) (input)
		NOTICE (user) (input)
		CTCP (user) (input)
		QUERY (user)


- Query
	Say(user, input);
	Act(user, input);
	Notice(user, input);
	CTCP(user, input);
	Whois(user);
	OpenQuer(user);
	CloseQuery(user);
	Invite(input, user);

	DCC
		DCCSend(user);
		DCCChat(user);
		DCCXmit(user);

IMenuProvider:
	DoMenu(MENU_TYPE_X, POINT pt, channel, user, input);

