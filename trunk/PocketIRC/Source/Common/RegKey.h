#ifndef _REGKEY_H_INCLUDED_
#define _REGKEY_H_INCLUDED_

class RegKey
{
public:
	RegKey(HKEY hk, const tstring& sSubKey);
	RegKey(RegKey& rk, const tstring& sSubKey);
	~RegKey();

	HRESULT Open(HKEY hk, const tstring& sSubKey);
	HRESULT Open(RegKey& rk, const tstring& sSubKey);
	HRESULT Clear();
	void Close();
	bool EnumKey(DWORD dwIndex, tstring& sKey);

	HKEY GetHKEY();

	void SetString(const tstring& sName, const tstring& sValue);
	tstring GetString(const tstring& sName, const tstring& sDefault);

	void SetDWORD(const tstring& sName, DWORD dwValue);
	DWORD GetDWORD(const tstring& sName, DWORD dwDefault);

	void SetBool(const tstring& sName, bool bValue);
	bool GetBool(const tstring& sName, bool bDefault);

private:
	HKEY m_hk;
};

#endif//_REGKEY_H_INCLUDED_
