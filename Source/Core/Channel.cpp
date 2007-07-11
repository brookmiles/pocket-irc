#include "PocketIRC.h"

#include "IrcString.h"

#include "Session.h"
#include "Channel.h"

/////////////////////////////////////////////////////////////////////////////
// Constructors/Destructors
/////////////////////////////////////////////////////////////////////////////

Channel::Channel()
{

}

Channel::~Channel()
{

}

/////////////////////////////////////////////////////////////////////////////
// Interface
/////////////////////////////////////////////////////////////////////////////

void Channel::SetName(const String& sName)
{
	m_sChannel = sName;
}


void Channel::ParseNameList(const String& stringIn)
{
	UINT iName = 0;
	String sName;

	while((sName = stringIn.GetWord(iName)).Size() > 0)
	{
		AddName(StripNick(sName), NickHasMode(sName, '@'), NickHasMode(sName, '+'));
		iName++;
	}
}

void Channel::AddName(const String& sName, bool bOp, bool bVoice)
{
	m_nickList.AddNick(sName, bOp, bVoice);
}

void Channel::RemoveName(const String& sName)
{
	m_nickList.RemoveNick(sName);
}

void Channel::ChangeName(const String& sOldName, const String& sNewName)
{
	NickListEntry* pEntry = m_nickList.GetEntry(sOldName);
	_ASSERTE(pEntry != NULL);

	if(pEntry != NULL)
	{
		pEntry->nick = sNewName;
	}
}

void Channel::OnMode(const NetworkEvent& event)
{
	_TRACE("Channel(0x%08X)::OnMode(%s)", this, event.GetParam(0).Str());

	Vector<ChannelMode> modeList;
	ParseChannelModes(event, modeList);

	for(UINT i = 0; i < modeList.Size(); ++i)
	{
		ChannelMode& mode = modeList[i];
		switch(mode.mode)
		{
		case 'v':
		case 'o':
			{
				String& sUser = mode.param;
				_ASSERTE(sUser.Size());

				if(sUser.Size())
				{
					NickListEntry* pEntry = m_nickList.GetEntry(sUser);
					_ASSERTE(pEntry != NULL);

					if(pEntry != NULL)
					{
						if(mode.mode == 'o')
						{
							pEntry->op = mode.add;
						}
						else
						{
							pEntry->voice = mode.add;
						}
					}
				}
			}
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// IChannel
/////////////////////////////////////////////////////////////////////////////

const String& Channel::GetName() const
{
	return m_sChannel;
}

bool Channel::IsOn(const String& sUser)
{
	return (m_nickList.Find(sUser) != -1);
}

bool Channel::IsVoice(const String& sUser)
{
	NickListEntry *pEntry = m_nickList.GetEntry(sUser);
	_ASSERTE(pEntry != NULL);

	if(pEntry != NULL)
	{
		return pEntry->voice;
	}
	return false;
}

bool Channel::IsOp(const String& sUser)
{
	NickListEntry *pEntry = m_nickList.GetEntry(sUser);
	_ASSERTE(pEntry != NULL);

	if(pEntry != NULL)
	{
		return pEntry->op;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// Utility
/////////////////////////////////////////////////////////////////////////////

void ParseChannelModes(const NetworkEvent& eventIn, Vector<ChannelMode>& listOut)
{
	UINT iModeParam = 1;
	UINT iParam = 1;


	while(iModeParam < eventIn.GetParamCount())
	{
		const String& sModeBlock = eventIn.GetParam(iModeParam);
		_ASSERTE(sModeBlock[0] == '+' || sModeBlock[0] == '-');

		UINT nModeBlockLen = sModeBlock.Size();
		bool bAdding = true;

		for(UINT i = 0; i < nModeBlockLen; ++i)
		{
			TCHAR cMode = sModeBlock[i];
			switch(cMode)
			{
				case '+':
				case '-':
					bAdding = (cMode == '+');
				break;
				case 'v':
				case 'o':
				{
					String sUser = eventIn.GetParam(iModeParam + iParam++);
					_ASSERTC(sUser.Size());
				
					ChannelMode mode = {bAdding, cMode, sUser};
					listOut.Append(mode);
				}
				break;
				case 'a': case 'i': case 'm': case 'n': case 'q': 
				case 'p': case 'r': case 's': case 't': 
				{
					ChannelMode mode = {bAdding, cMode};
					listOut.Append(mode);
				}
				break;
				case 'k': case 'l':
				{
					// Add takes a parameter, remove doesn't

					String sParam;
					if(bAdding)
					{
						sParam = eventIn.GetParam(iModeParam + iParam++);
						_ASSERTC(sParam.Size());
					}

					ChannelMode mode = {bAdding, cMode, sParam};
					listOut.Append(mode);
				}
				break;
				case 'b': case 'e': case 'I': case 'O':
				{
					// Adding and remove both take parameters

					String sParam = eventIn.GetParam(iModeParam + iParam++);
					_ASSERTC(sParam.Size());

					ChannelMode mode = {bAdding, cMode, sParam};
					listOut.Append(mode);
				}
				break;
				default:
					_TRACE("... UNKNOWN MODE CHARACTER: %c", cMode);
			}
		}			

		// Next block of modes if available
		iModeParam += iParam;
		iParam = 1;
	}
}
