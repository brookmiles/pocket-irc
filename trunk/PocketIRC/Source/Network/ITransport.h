#ifndef _TRANSPORT_H_INCLUDED_
#define _TRANSPORT_H_INCLUDED_

#include "StringT.h"


class ITransportReader
{
public:
	virtual void OnConnect(HRESULT hr, LPCTSTR pszError) = 0;
	virtual void OnError(HRESULT hr) = 0;
	virtual void OnClose() = 0;
	virtual void OnRead(BYTE *pData, UINT nSize) = 0;
};

class ITransportWrite
{
public:
	virtual HRESULT Write(BYTE *pData, UINT nSize) = 0;
};

class ITransport
{
public:
	virtual HRESULT Connect(const String& sHostName, USHORT uPort) = 0;
	virtual HRESULT Close() = 0;
	virtual bool IsOpen() = 0;
	virtual String GetLocalAddress() = 0;
};

class ITransportListen
{
public:
	virtual HRESULT Listen(USHORT uPort) = 0;
	virtual HRESULT Accept() = 0;
	virtual HRESULT Close() = 0;
	virtual bool IsOpen() = 0;
};


#endif//_TRANSPORT_H_INCLUDED_
