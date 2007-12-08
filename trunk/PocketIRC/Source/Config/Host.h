#ifndef _HOST_H_INCLUDED_
#define _HOST_H_INCLUDED_

class Host
{
public:
	Host();
	Host(Host& hostCopyFrom);
	Host(const tstring& sAddress, USHORT usPort);
	~Host();

	const Host& operator=(Host& hostCopyFrom);

	void SetAddress(const tstring& sAddress);
	const tstring& GetAddress();

	void SetPort(USHORT usPort);
	USHORT GetPort();

	void SetPass(const tstring& sPass);
	const tstring& GetPass();

private:
	tstring m_sAddress;
	USHORT m_usPort;
	tstring m_sPass;
};
	
#endif//_HOST_H_INCLUDED_
