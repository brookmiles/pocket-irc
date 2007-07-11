#include "PocketIRC.h"
#include "Format.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

Format::Format()
{
	m_idEvent = 0;
	m_bIncoming = true;
	m_bEnabled = true;
}

Format::Format(Format& from)
{
	*this = from;
}

Format::~Format()
{

}

/////////////////////////////////////////////////////////////////////////////
//	Operators
/////////////////////////////////////////////////////////////////////////////

const Format& Format::operator=(Format& from)
{
	SetEventName(from.GetEventName());
	SetEventID(from.GetEventID());
	SetFormat(from.GetFormat());
	SetIncoming(from.IsIncoming());
	SetEnabled(from.IsEnabled());

	return *this;
}
