#include "PocketIRC.h"
#include "RegKey.h"

/////////////////////////////////////////////////////////////////////////////
// Constructors/Destructors
/////////////////////////////////////////////////////////////////////////////

RegKey::RegKey(HKEY hk, const tstring& sSubKey)
{
//	_TRACE("RegKey(0x%08X)::RegKey(0x%08X, \"%s\")", this, hk, sSubKey.c_str());

	m_hk = NULL;

	HRESULT hr = Open(hk, sSubKey);
//	_TRACE("... Open() returned 0x%08X", hr);
}

RegKey::RegKey(RegKey& rk, const tstring& sSubKey)
{
//	_TRACE("RegKey(0x%08X)::RegKey(0x%08X, \"%s\")", this, rk.GetHKEY(), sSubKey.c_str());

	m_hk = NULL;

	HRESULT hr = Open(rk, sSubKey);
//	_TRACE("... Open() returned 0x%08X", hr);
}

RegKey::~RegKey()
{
	Close();
}


/////////////////////////////////////////////////////////////////////////////
// Interface
/////////////////////////////////////////////////////////////////////////////

HRESULT RegKey::Open(HKEY hk, const tstring& sSubKey)
{
	DWORD dwCreate = 0;
	LONG err = RegCreateKeyEx(hk, sSubKey.c_str(), 0, NULL, 0, 0, NULL, &m_hk, &dwCreate);
	return HRESULT_FROM_WIN32(err);
}

HRESULT RegKey::Open(RegKey& rk, const tstring& sSubKey)
{
	DWORD dwCreate = 0;
	LONG err = RegCreateKeyEx(rk.GetHKEY(), sSubKey.c_str(), 0, NULL, 0, 0, NULL, &m_hk, &dwCreate);
	return HRESULT_FROM_WIN32(err);
}

void RegKey::Close()
{
	if(m_hk)
	{
		RegCloseKey(m_hk);
		m_hk = NULL;
	}
}

HRESULT RegKey::Clear()
{
	TCHAR buf[POCKETIRC_MAX_REG_SZ_LEN + 1];
	DWORD dwSize = sizeof(buf)/sizeof(buf[0]);
	HRESULT hr = S_OK;

	while(SUCCEEDED(hr) && (RegEnumKeyEx(m_hk, 0, buf, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS))
	{
		LONG lErr = RegDeleteKey(m_hk, buf);
		hr = HRESULT_FROM_WIN32(lErr);

		dwSize = sizeof(buf);
	}

	while(SUCCEEDED(hr) && (RegEnumValue(m_hk, 0, buf, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS))
	{
		LONG lErr = RegDeleteValue(m_hk, buf);
		hr = HRESULT_FROM_WIN32(lErr);

		dwSize = sizeof(buf);
	}

	return hr;
}

bool RegKey::EnumKey(DWORD dwIndex, tstring& sKey)
{
	TCHAR buf[POCKETIRC_MAX_REG_SZ_LEN + 1];
	DWORD dwSize = sizeof(buf)/sizeof(buf[0]);

	ZeroMemory(buf, sizeof(buf));

	LONG lErr = RegEnumKeyEx(m_hk, dwIndex, buf, &dwSize, NULL, NULL, NULL, NULL);
	buf[POCKETIRC_MAX_REG_SZ_LEN] = '\0';

	sKey = buf;

	return (lErr == ERROR_SUCCESS);
}

HKEY RegKey::GetHKEY()
{
	return m_hk;
}


void RegKey::SetString(const tstring& sName, const tstring& sValue)
{
//	_TRACE("RegKey(0x%08X)::SetString(\"%s\", \"%s\")", this, sName.c_str(), sValue.c_str());

	LONG lErr = RegSetValueEx(m_hk, sName.c_str(), 0, REG_SZ, (BYTE*)sValue.c_str(), 
		(sValue.size() + 1) * sizeof(TCHAR));

//	_TRACE("...RegSetValueEx() returned 0x%08X", lErr);
}

tstring RegKey::GetString(const tstring& sName, const tstring& sDefault)
{
	DWORD dwType = 0;
	TCHAR buf[POCKETIRC_MAX_REG_SZ_LEN + 1];
	DWORD dwSize = sizeof(buf);
	
	ZeroMemory(buf, sizeof(buf));

	LONG lErr = RegQueryValueEx(m_hk, sName.c_str(), 0, &dwType, (BYTE*)buf, &dwSize);
	if(lErr != ERROR_SUCCESS)
	{
		return sDefault;
	}
	else
	{
		buf[POCKETIRC_MAX_REG_SZ_LEN] = '\0';
		return buf;
	}
}


void RegKey::SetDWORD(const tstring& sName, DWORD dwValue)
{
//	_TRACE("RegKey(0x%08X)::SetDWORD(\"%s\", %u)", this, sName.c_str(), dwValue);

	LONG lErr = RegSetValueEx(m_hk, sName.c_str(), 0, REG_DWORD, (BYTE*)&dwValue, 
		sizeof(DWORD));

//	_TRACE("...RegSetValueEx() returned 0x%08X", lErr);
}

DWORD RegKey::GetDWORD(const tstring& sName, DWORD dwDefault)
{
	DWORD dwType = 0;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwValue = 0;
	
	LONG lErr = RegQueryValueEx(m_hk, sName.c_str(), 0, &dwType, (BYTE*)&dwValue, &dwSize);
	if(lErr != ERROR_SUCCESS)
	{
		return dwDefault;
	}
	else
	{
		return dwValue;
	}
}


void RegKey::SetBool(const tstring& sName, bool bValue)
{
	SetDWORD(sName, (DWORD)bValue);
}

bool RegKey::GetBool(const tstring& sName, bool bDefault)
{
	return GetDWORD(sName, (DWORD)bDefault) != 0;
}
