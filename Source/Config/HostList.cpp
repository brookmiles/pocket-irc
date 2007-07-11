#include "PocketIRC.h"
#include "HostList.h"

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

HostList::HostList()
{
//	_TRACE("HostList(0x%08X)::HostList()", this);
	m_pDefault = NULL;
}

HostList::HostList(HostList& from)
{
//	_TRACE("HostList(0x%08X)::HostList(0x%08X)", this, &from);
	m_pDefault = NULL;
	*this = from;
}


HostList::~HostList()
{
//	_TRACE("HostList(0x%08X)::~HostList()", this);

	if(m_listHosts.Size() > 0)
	{
		Clear();
	}

	m_listHosts.Free();
}

/////////////////////////////////////////////////////////////////////////////
//	Operators
/////////////////////////////////////////////////////////////////////////////

const HostList& HostList::operator=(HostList& from)
{
	Clear();

	for(UINT i = 0; i < from.Count(); ++i)
	{
		Host* pHost = NULL;

		HRESULT hr = from.Item(i, &pHost);
		_ASSERTE(SUCCEEDED(hr));
		_ASSERTE(pHost != NULL);

		Host* pNewHost = NULL;
		hr = AddHost(pHost, &pNewHost);
		_ASSERTE(SUCCEEDED(hr));
		_ASSERTE(pNewHost != NULL);

		if(pHost == from.GetDefault())
		{
			SetDefault(pNewHost);
		}
	}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

HRESULT HostList::FindHost(const String& sAddress, Host** ppFoundHost)
{
	_ASSERTE(ppFoundHost != NULL);

//	_TRACE("HostList(0x%08X)::FindHost(%s, 0x%08X)", this, sAddress.Str(), ppFoundHost);

	HRESULT hr = S_FALSE;

	for(UINT i = 0; i < m_listHosts.Size(); ++i)
	{
		Host* pHost = m_listHosts[i];
		_ASSERTE(pHost != NULL);

		if(sAddress.Compare(pHost->GetAddress(), false))
		{
			*ppFoundHost = pHost;
			hr = S_OK;

			break;
		}
	}
	return hr;
}

HRESULT HostList::AddHost(const String& sAddress, USHORT usPort, Host** ppNewHost)
{
	_ASSERTE(ppNewHost != NULL);

//	_TRACE("HostList(0x%08X)::AddHost(%s, %u, 0x%08X)", this, sAddress.Str(), usPort, ppNewHost);

	*ppNewHost = new Host(sAddress, usPort);
	_ASSERTE(*ppNewHost != NULL);

	m_listHosts[m_listHosts.Size()] = *ppNewHost;

	return S_OK;
}

HRESULT HostList::AddHost(Host* pHost, Host** ppNewHost)
{
	_ASSERTE(pHost != NULL);
	_ASSERTE(ppNewHost != NULL);

//	_TRACE("HostList(0x%08X)::AddHost(0x%08X, 0x%08X)", this, pHost, ppNewHost);

	*ppNewHost = new Host(*pHost);
	_ASSERTE(*ppNewHost != NULL);

	m_listHosts[m_listHosts.Size()] = *ppNewHost;

	return S_OK;
}

HRESULT HostList::RemoveHost(Host* pHost)
{
//	_TRACE("HostList(0x%08X)::RemoveHost(0x%08X)", this, pHost);
	_ASSERTE(pHost != NULL);

	HRESULT hr = E_INVALIDARG;

	for(UINT i = 0; i < m_listHosts.Size(); ++i)
	{
		if(m_listHosts[i] == pHost)
		{
			bool bEraseOk = m_listHosts.Erase(i);
			_ASSERTE(bEraseOk);

			if(pHost == m_pDefault)
			{
				m_pDefault = NULL;
			}

			delete pHost;
			hr = S_OK;

			break;
		}
	}
	return hr;
}

void HostList::Clear()
{
//	_TRACE("HostList(0x%08X)::Clear()", this);

	for(UINT i = 0; i < m_listHosts.Size(); ++i)
	{
		delete m_listHosts[i];
	}

	m_listHosts.Shrink(0);
}


UINT HostList::Count()
{
//	_TRACE("HostList(0x%08X)::Count()", this);
	return m_listHosts.Size();
}

HRESULT HostList::Item(UINT nIndex, Host** ppHost)
{
//	_TRACE("HostList(0x%08X)::Item(%u, 0x%08X)", this, nIndex, ppHost);
	_ASSERTE(ppHost != NULL);
	
	HRESULT hr = E_INVALIDARG;
	
	if(nIndex < m_listHosts.Size())
	{
		*ppHost = m_listHosts[nIndex];
		hr = S_OK;
	}
	return hr;
}

void HostList::SetDefault(Host* pDefault)
{
	m_pDefault = pDefault;
}

Host* HostList::GetDefault()
{
	return m_pDefault;
}
