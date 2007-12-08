#include "PocketIRC.h"

#include "IrcString.h"
#include "StringUtil.h"

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

void Channel::SetName(const tstring& sName)
{
	m_sChannel = sName;
}

void Channel::ParseNameList(const tstring& stringIn)
{
	std::vector<tstring> names;
	Split(stringIn, names, _T(' '), false);

	for(std::vector<tstring>::iterator i = names.begin(); i != names.end(); ++i)
	{
		AddName(StripNick(*i, _T("@+")), NickHasMode(*i, '@'), NickHasMode(*i, '+'));
	}
}

void Channel::AddName(const tstring& sName, bool bOp, bool bVoice)
{
	m_nickList.AddNick(sName, bOp, bVoice);
}

void Channel::RemoveName(const tstring& sName)
{
	m_nickList.RemoveNick(sName);
}

void Channel::ChangeName(const tstring& sOldName, const tstring& sNewName)
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
	_TRACE("Channel(0x%08X)::OnMode(%s)", this, event.GetParam(0).c_str());

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

const tstring& Channel::GetName() const
{
	return m_sChannel;
}

bool Channel::IsOn(const tstring& sUser)
{
	return (m_nickList.Find(sUser) != -1);
}

bool Channel::IsVoice(const tstring& sUser)
{
	NickListEntry *pEntry = m_nickList.GetEntry(sUser);
	_ASSERTE(pEntry != NULL);

	if(pEntry != NULL)
	{
		return pEntry->voice;
	}
	return false;
}

bool Channel::IsOp(const tstring& sUser)
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

void ParseChannelModes(const NetworkEvent& eventIn, std::vector<ChannelMode>& listOut)
{
	UINT iModeParam = 1;
	UINT iParam = 1;


	while(iModeParam < eventIn.GetParamCount())
	{
		const tstring& sModeBlock = eventIn.GetParam(iModeParam);
		_ASSERTE(sModeBlock[0] == '+' || sModeBlock[0] == '-');

		UINT nModeBlockLen = sModeBlock.size();
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
					tstring sUser = eventIn.GetParam(iModeParam + iParam++);
					_ASSERTC(sUser.size());
				
					ChannelMode mode = {bAdding, cMode, sUser};
					listOut.push_back(mode);
				}
				break;
				case 'a': case 'i': case 'm': case 'n': case 'q': 
				case 'p': case 'r': case 's': case 't': 
				{
					ChannelMode mode = {bAdding, cMode};
					listOut.push_back(mode);
				}
				break;
				case 'k': case 'l':
				{
					// Add takes a parameter, remove doesn't

					tstring sParam;
					if(bAdding)
					{
						sParam = eventIn.GetParam(iModeParam + iParam++);
						_ASSERTC(sParam.size());
					}

					ChannelMode mode = {bAdding, cMode, sParam};
					listOut.push_back(mode);
				}
				break;
				case 'b': case 'e': case 'I': case 'O':
				{
					// Adding and remove both take parameters

					tstring sParam = eventIn.GetParam(iModeParam + iParam++);
					_ASSERTC(sParam.size());

					ChannelMode mode = {bAdding, cMode, sParam};
					listOut.push_back(mode);
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
