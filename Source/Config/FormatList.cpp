#include "PocketIRC.h"
#include "FormatList.h"

#include "Core\NetworkEvent.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

FormatList::FormatList()
{

}

FormatList::FormatList(FormatList& from)
{
	*this = from;
}

FormatList::~FormatList()
{
	Clear();
}

/////////////////////////////////////////////////////////////////////////////
//	Operators
/////////////////////////////////////////////////////////////////////////////

const FormatList& FormatList::operator=(FormatList& from)
{
	Clear();

	UINT nFormats = from.Count();
	for(UINT i = 0; i < nFormats; ++i)
	{
		Format* pFrom = from.Item(i);
		_ASSERTE(pFrom != NULL);

		Format* pNew = new Format(*pFrom);
		AddFormat(pNew);
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

Format* FormatList::GetFormat(int idEvent, bool bIncoming)
{
	UINT nFormats = m_vecFormats.size();
	for(UINT i = 0; i < nFormats; ++i)
	{
		Format* pFormat = m_vecFormats[i];
		_ASSERTE(pFormat != NULL);

		if(pFormat->GetEventID() == idEvent && pFormat->IsIncoming() == bIncoming)
		{
			return pFormat;
		}
	}

	if(NetworkEvent::IsSystemEvent(idEvent))
	{
		// System events are never marked incoming, since they are local
		return GetFormat(SYS_EVENT_UNKNOWN, false);
	}
	else if(NetworkEvent::IsReplyEvent(idEvent))
	{
		// Server reply events are always incoming, they are never sent
		return GetFormat(IRC_RPL_UNKNOWN, true);
	}
	else if(NetworkEvent::IsCTCPEvent(idEvent))
	{
		return GetFormat(IRC_CTCP_UNKNOWN, bIncoming);
	}
	else if(NetworkEvent::IsCTCPReplyEvent(idEvent))
	{
		return GetFormat(IRC_CTCP_RPL_UNKNOWN, bIncoming);
	}
	else
	{
		return GetFormat(IRC_CMD_UNKNOWN, bIncoming);
	}
	return NULL;
}

void FormatList::AddFormat(Format* pFormat)
{
	m_vecFormats.push_back(pFormat);
}

UINT FormatList::Count()
{
	return m_vecFormats.size();
}

Format* FormatList::Item(UINT nIndex)
{
	return m_vecFormats[nIndex];
}

void FormatList::Clear()
{
	UINT nFormats = m_vecFormats.size();
	for(UINT i = 0; i < nFormats; ++i)
	{
		delete m_vecFormats[i];
	}
	m_vecFormats.clear();
}
