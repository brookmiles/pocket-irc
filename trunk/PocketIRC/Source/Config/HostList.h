#ifndef _HOSTLIST_H_INCLUDED_
#define _HOSTLIST_H_INCLUDED_

#include "Host.h"
#include "Vector.h"
#include "StringT.h"

class HostList
{
public:
	HostList(HostList& from);
	HostList();
	~HostList();

	const HostList& operator=(HostList& from);

	HRESULT FindHost(const String& sAddress, Host** ppNewHost);
	HRESULT AddHost(const String& sAddress, USHORT usPort, Host** ppNewHost);
	HRESULT AddHost(Host* pHost, Host** ppNewHost);
	HRESULT RemoveHost(Host* pHost);
	void Clear();

	void SetDefault(Host* pDefault);
	Host* GetDefault();

	UINT Count();
	HRESULT Item(UINT nIndex, Host** ppHost);

private:
	Vector<Host*> m_listHosts;
	Host* m_pDefault;
};

#endif//_HOSTLIST_H_INCLUDED_
