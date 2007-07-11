#ifndef _HOST_H_INCLUDED_
#define _HOST_H_INCLUDED_

#include "StringT.h"

class Host
{
public:
	Host();
	Host(Host& hostCopyFrom);
	Host(const String& sAddress, USHORT usPort);
	~Host();

	const Host& operator=(Host& hostCopyFrom);

	void SetAddress(const String& sAddress);
	const String& GetAddress();

	void SetPort(USHORT usPort);
	USHORT GetPort();

	void SetPass(const String& sPass);
	const String& GetPass();

private:
	String m_sAddress;
	USHORT m_usPort;
	String m_sPass;
};
	
#endif//_HOST_H_INCLUDED_
