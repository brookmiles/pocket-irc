#include "PocketIRC.h"
#include "Host.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

Host::Host()
{
	//_TRACE("Host(0x%08X)::Host()", this);

	m_usPort = POCKETIRC_DEFAULT_PORT;
}

Host::Host(Host& hostCopyFrom)
{
	//_TRACE("Host(0x%08X)::Host(0x%08X)", this, &hostCopyFrom);

	// SetAddress() method counts on members being initialized
	m_usPort = 0;

	*this = hostCopyFrom;
}

Host::Host(const String& sAddress, USHORT usPort)
{
	//_TRACE("Host(0x%08X)::Host(0x%08X, %u)", this, pszAddress, usPort);

	// SetAddress() method counts on members being initialized
	m_usPort = 0;

	SetAddress(sAddress);
	SetPort(usPort);
}

Host::~Host()
{
	//_TRACE("Host(0x%08X)::~Host()", this);
}

/////////////////////////////////////////////////////////////////////////////
//	Operators
/////////////////////////////////////////////////////////////////////////////

const Host& Host::operator=(Host& hostCopyFrom)
{
	//_TRACE("Host(0x%08X)::operator=(0x%08X)", this, &hostCopyFrom);

	SetAddress(hostCopyFrom.GetAddress());
	SetPort(hostCopyFrom.GetPort());
	SetPass(hostCopyFrom.GetPass());

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
//	Accessor Methods
/////////////////////////////////////////////////////////////////////////////

void Host::SetAddress(const String& sAddress)
{
	//_TRACE("Host(0x%08X)::SetAddress(0x%08X)", this, pszAddress);
	
	m_sAddress = sAddress;
}

const String& Host::GetAddress()
{
	//_TRACE("Host(0x%08X)::GetAddress()", this);
	return m_sAddress;
}


void Host::SetPort(USHORT usPort)
{
	//_TRACE("Host(0x%08X)::SetPort(%u)", this, usPort);
	m_usPort = usPort;
}

USHORT Host::GetPort()
{
	//_TRACE("Host(0x%08X)::GetPort()", this);
	return m_usPort;
}

void Host::SetPass(const String& sPass)
{
	//_TRACE("Host(0x%08X)::SetPass(0x%08X)", this, pszPass);
	
	m_sPass = sPass;
}

const String& Host::GetPass()
{
	//_TRACE("Host(0x%08X)::GetPass()", this);
	return m_sPass;
}
