#ifndef _DCCSEND_H_INCLUDED_
#define _DCCSEND_H_INCLUDED_

#include "Network\SocketTransport.h"
#include "Network\SocketAccepter.h"

#include "DCCHandler.h"
#include "IDCCSession.h"

class DCCSend : 
	public IDCCSession, 
	public ITransportReader, 
	public ISocketAccepter
{
public:
	DCCSend();
	~DCCSend();

	void SetDCCHandler(DCCHandler* pDCCHandler) { m_pDCCHandler = pDCCHandler; }

	void IncomingRequest(const tstring& sRemoteUser, ULONG ulRemoteAddress, USHORT ulRemotePort, const tstring& sFileName, ULONG ulFileSize);
	bool OutgoingRequest(const tstring& sRemoteUser);

	tstring GetFileName() { return m_sFileName; }
	ULONG GetFileSize() { return m_ulFileSize; }

	bool Resume(ULONG ulSize);
	void Resume() { Connect(); }

// IDCCSession
	DCC_TYPE GetType() { return DCC_SEND; }
	DCC_STATE GetState() { return m_state; }
	bool IsIncoming() { return m_bIncoming; }

	void Accept();
	void Close();

	tstring GetRemoteUser();
	tstring GetRemoteHost() { return m_sRemoteHost; }
	USHORT GetRemotePort() { return m_uRemotePort; }
	USHORT GetLocalPort() { return m_uLocalPort; }

	tstring GetStateString();
	tstring GetDescription();

// ITransportReader
	void OnConnect(HRESULT hr, LPCTSTR pszError);
	void OnError(HRESULT hr);
	void OnClose();
	void OnRead(BYTE* pData, UINT nSize);

// ISocketAccepter
	bool OnAccept(HRESULT hr, SocketTransport* pTransport, LPCTSTR psz);

private:
	bool DoAcceptFileName();
	bool DoSendFileName();
	void CloseFile(bool bDelete = false);
	void CloseSendThread();
	void Connect();

	static DWORD CALLBACK SendThreadStub(LPVOID lpv);
	DWORD SendThreadProc();

	DCCHandler* m_pDCCHandler;

	DCC_STATE m_state;
	bool m_bIncoming;

	tstring m_sRemoteUser;
	tstring m_sRemoteHost;
	ULONG m_ulRemoteAddress;
	USHORT m_uRemotePort;
	USHORT m_uLocalPort;

	tstring m_sFileName;
	ULONG m_ulFileSize;

	SocketAccepter m_sockAccepter;
	SocketTransport* m_pTransport;

	ULONG m_nTransferred;
	HANDLE m_hFile;

	HTHREAD m_hSendThread;
};

#endif//_DCCSEND_H_INCLUDED_
