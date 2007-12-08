#ifndef _HOSTLIST_H_INCLUDED_
#define _HOSTLIST_H_INCLUDED_

#include "Host.h"

class HostList
{
public:
	HostList(HostList& from);
	HostList();
	~HostList();

	const HostList& operator=(HostList& from);

	HRESULT FindHost(const tstring& sAddress, Host** ppNewHost);
	HRESULT AddHost(const tstring& sAddress, USHORT usPort, Host** ppNewHost);
	HRESULT AddHost(Host* pHost, Host** ppNewHost);
	HRESULT RemoveHost(Host* pHost);
	void Clear();

	void SetDefault(Host* pDefault);
	Host* GetDefault();

	UINT Count();
	HRESULT Item(UINT nIndex, Host** ppHost);

private:
	typedef std::vector<Host*> listtype_t;
	listtype_t m_listHosts;
	Host* m_pDefault;
};

#endif//_HOSTLIST_H_INCLUDED_
