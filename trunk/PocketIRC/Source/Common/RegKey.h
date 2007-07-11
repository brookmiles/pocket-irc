#ifndef _REGKEY_H_INCLUDED_
#define _REGKEY_H_INCLUDED_

#include "StringT.h"

class RegKey
{
public:
	RegKey(HKEY hk, const String& sSubKey);
	RegKey(RegKey& rk, const String& sSubKey);
	~RegKey();

	HRESULT Open(HKEY hk, const String& sSubKey);
	HRESULT Open(RegKey& rk, const String& sSubKey);
	HRESULT Clear();
	void Close();
	bool EnumKey(DWORD dwIndex, String& sKey);

	HKEY GetHKEY();

	void SetString(const String& sName, const String& sValue);
	String GetString(const String& sName, const String& sDefault);

	void SetDWORD(const String& sName, DWORD dwValue);
	DWORD GetDWORD(const String& sName, DWORD dwDefault);

	void SetBool(const String& sName, bool bValue);
	bool GetBool(const String& sName, bool bDefault);

private:
	HKEY m_hk;
};

#endif//_REGKEY_H_INCLUDED_
