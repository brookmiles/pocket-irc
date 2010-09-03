#include <windows.h>
#include <string>
#include "StringConv.h"

std::string WideToMB(const std::wstring& str, UINT codePage) 
{
	std::string ret;
	if(str.length() > 0)
	{
		DWORD mbChars = WideCharToMultiByte(codePage, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
		_ASSERTE(mbChars > 0);
		if(mbChars > 0)
		{
			char* buf = new char[mbChars];
			_ASSERTE( buf != NULL );
			if( buf != NULL )
			{
				ZeroMemory(buf, mbChars);

				DWORD charsConverted = WideCharToMultiByte(codePage, 0, str.c_str(), -1, buf, mbChars, NULL, NULL);
				_ASSERTE( charsConverted > 0 );
				_ASSERTE( charsConverted <= mbChars );

				buf[mbChars - 1] = 0;
				ret = buf;

				delete[] buf;
			}
		}
	}
	return ret;
}

std::wstring MBToWide(const std::string& str, UINT codePage) 
{
	std::wstring ret;
	if(str.length() > 0)
	{
		DWORD wChars = MultiByteToWideChar(codePage, 0, str.c_str(), -1, NULL, 0);
		_ASSERTE(wChars > 0);
		if(wChars > 0)
		{
			wchar_t* buf = new wchar_t[wChars];
			_ASSERTE( buf != NULL );
			if( buf != NULL )
			{
				size_t bytesNeeded = sizeof(wchar_t)*wChars;
				ZeroMemory(buf, bytesNeeded);

				DWORD charsConverted = MultiByteToWideChar(codePage, 0, str.c_str(), -1, buf, wChars);
				_ASSERTE( charsConverted > 0 );
				_ASSERTE( charsConverted <= wChars );

				buf[wChars - 1] = 0;
				ret = buf;

				delete[] buf;
			}
		}
	}
	return ret;
}

