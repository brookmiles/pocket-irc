You must have Microsoft ActiveSync or Windows Mobile Device Center installed on this computer in order to install Pocket IRC. This program will copy Pocket IRC files to your computer and launch ActiveSync to install them on your device.

o Pocket PC 2003 or Windows Mobile 5 or later device with touchscreen is required
o Version 1.1 supports Pocket PC 2000/2002 devices, available from http://pocketirc.com/

Version History

1.3.1 (2010-09-02)
o NEW: UTF8 encoding option
o FIXED: URL context menu works again
o FIXED: Improved nick detection for user context menu

1.3 (2010-06-18)
o NEW: PocketIRC is now FREE, and a registration code is no longer required
o NEW: When the input box is empty, the left and right arrows will navigate between tabs (added by Roger Clark)
o NEW: Recognize some server capability strings such as channel and user modes
o FIXED: Some other bugs I don't remember
o CHANGED: Internal refactoring to use standard string classes

1.2.5 (2007-11-27)
o FIXED: Query windows incorrectly sent messages to the old nick after a user changed their nickname
o FIXED: Incorrect handling of user names containing non-breaking spaces

1.2.4 (2007-07-07)
o NEW: Landscape and Square mode screen support
o NEW: Optional message timestamps
o NEW: Server channel listing and improved channel favourites
o NEW: Channel windows remain open on kick, quit or disconnection, allowing easy rejoin and no loss of text
o NEW: More / commands implemented, some default behaviour changed (user specified aliases coming eventually)
o NEW: Format messages for more error events (banned, not on channel, channel limit reached, etc.)
o NEW: Disconnection is indicated on channel windows as well as the Status window
o NEW: Ident and host format tags $i and $h
o FIXED: Use Full Screen option sometimes did not hide the top menu bar immediately
o FIXED: Attempting to querying users with non-standard characters in their nickname did nothing
o REMOVED: Exit on minimize option, you can exit completely from the IRC menu or by typing /exit

1.2.3 (2007-02-24)
o NEW: Basic keyboard '/' command processing in addition to the menu interface. /join, /nick, /msg, etc.

1.2.2 (2007-02-18)
o NEW: Favourite Channels list
o NEW: Auto-Join Favourite Channels on connect
o NEW: Ping messages will be sent to the server after 30 seconds of inactivity in order to properly detect disconnection.
o NEW: Network errors now provide descriptive messages, not just error codes
o FIXED: Pressing up arrow now selects the last thing entered, not the first

1.2.1 (2006-09-07)
o FIXED: IRC display will now honour the system ClearType setting, instead of always using ClearType

1.2 (2006-09-02)
o NEW: Support "Inverse" colour code
o NEW: Attempt to establish internet connection automatically before connecting
o FIXED: Better support for VGA devices and Windows Mobile 5
o FIXED: Sending messages starting with : would have the : removed in certain cases
o FIXED: Listing fonts with random styles in some situations
o FIXED: Text formatted incorrectly after changing the font size
o FIXED: Erronously displayed hostname as IP in DCC options on some servers
o FIXED: Work around for some Bahamut (DALNet) daemons' poor line buffering

1.1 (2004-02-07)
o NEW: DCC SEND and CHAT commands
o NEW: DCC RESUME capability for send and receive
o CHANGED: Send IDENT Username in USER command instead of Nickname during login

1.0.7 (2004-01-20)
o FIXED: Inability to open queries to services containing an @ symbol
o FIXED: Commands not executing if there are extra spaces surrounding nick/channel names
o FIXED: Obscure problem due to ezbounce proxy not parsing commands properly

1.0.6 (2003-02-21)
o NEW: Tap-n-hold popup menu for usernames on channel windows
o NEW: Tap-n-hold popup menu for channels and URLs
o NEW: Copy words from display to input box by tapping
o NEW: Incoming DCC File Transfer and Chat (sending is not yet supported)
o FIXED: CTCP sending did not parse input or send message correctly
o FIXED: Crash when incoming CTCP message has no terminator
o FIXED: Not being able to join channels if nickname was longer than the server accepts
o FIXED: Program sometimes not closing properly under PocketBlinds
o FIXED: Tapping display windows removes focus from input box
o FIXED: Speed optimisations, redundant drawing reduced
o FIXED: May not recognise nicknames containing a ^ character
o FIXED: Cursor doesn't show up on Server list in Options dialog untill you type something
o CHANGED: Quit Message and Real Name options are disabled in unregistered version

1.0.5 (2002-10-22)
o NEW: Tap-and-hold Context Menu on tabs
o NEW: Channel user list and popup menu
o NEW: Proper word wrapping
o NEW: More commands added (Op, Voice, NickServ, etc...)
o NEW: More events added to formatting list
o FIXED: Bug which prevented Context Menu from being displayed in certain cases
o FIXED: Bug which improperly matched events with the wrong window in certain cases
o FIXED: Selections sometimes copied more than was selected to the clipboard
o CHANGED: Removed Help item on IRC menu, use Help on the main Start Menu instead
o CHANGED: IRC menu now always displays Server Menu instead of Context Menu
o CHANGED: Updated documentation

1.0.4 (2002-10-11)
o NEW: Added support for server PASS command
o CHANGED: Show NICK changes in each channel window a user is in
o CHANGED: Show QUIT messages in each channel window a user was in
o CHANGED: Display full version and build number in various places

1.0.3
o NEW: Support for all Pocket PC CPUs (ARM, SH3, MIPS)

1.0.2
o NEW: Join command supports specifying a key for channels which require one
o CHANGED: Update help and installer to reference pocketirc.com

1.0.1 (2002-09-28)
o Initial Release
