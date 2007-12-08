#include "PocketIRC.h"
#include "DCCSend.h"

#include "Config\Options.h"
#include "IrcString.h"

static const TCHAR validchars[] = _T("abcdefghijklmnopqsrtuvwxyzABCDEFGHIJKLMNOPQSRTUVWXYZ1234567890_-.");

static tstring SafeDCCFileName(const tstring& sFileName)
{
	tstring sNewName;

	TCHAR* pszLastSlash = _tcsrchr(sFileName.c_str(), '\\');
	if(pszLastSlash)
	{
		sNewName = pszLastSlash + 1;
	}
	else
	{
		sNewName = sFileName;
	}

	UINT nSize = sNewName.size();
	for(UINT i = 0; i < nSize; ++i)
	{
		if(!_tcschr(validchars, sNewName[i]))
		{
			sNewName[i] = '_';
		}
	}

	return sNewName;
}

/////////////////////////////////////////////////////////////////////////////
//	Constructor and Destructor
/////////////////////////////////////////////////////////////////////////////

DCCSend::DCCSend()
{
	_TRACE("DCCSend(0x%08X)::DCCSend()", this);

	m_pDCCHandler = NULL;

	m_state = DCC_STATE_ERROR;
	m_bIncoming = false;

	m_uRemotePort = 0;
	m_uLocalPort = 0;

	m_ulFileSize = 0;

	m_pTransport = NULL;
	m_sockAccepter.SetAccepter(this);
	m_sockAccepter.SetNotify(ISocketNotify::Instance);

	m_nTransferred = 0;
	m_hFile = INVALID_HANDLE_VALUE;

	m_hSendThread = NULL;
}

DCCSend::~DCCSend()
{
	_TRACE("DCCSend(0x%08X)::~DCCSend()", this);

	delete m_pTransport;
	CloseFile();
}

/////////////////////////////////////////////////////////////////////////////
//	Interface
/////////////////////////////////////////////////////////////////////////////

void DCCSend::IncomingRequest(const tstring& sRemoteUser, ULONG ulRemoteAddress, USHORT ulRemotePort, 
	const tstring& sFileName, ULONG ulFileSize)
{
	USES_CONVERSION;

	m_state = DCC_STATE_REQUEST;
	m_bIncoming = true;

	IN_ADDR inad;
	inad.S_un.S_addr = htonl(ulRemoteAddress);

	m_sRemoteHost = A2CT(inet_ntoa(inad));
	m_sRemoteUser = sRemoteUser;
	m_ulRemoteAddress = ulRemoteAddress;
	m_uRemotePort = ulRemotePort;

	m_sFileName = sFileName;
	m_ulFileSize = ulFileSize;

}

bool DCCSend::OutgoingRequest(const tstring& sRemoteUser)
{
	if(DoSendFileName())
	{
		USES_CONVERSION;

		m_bIncoming = false;
		m_sRemoteUser = sRemoteUser;

		HRESULT hr = E_UNEXPECTED;

		for(USHORT usPort = g_Options.GetDCCStartPort(); usPort <= g_Options.GetDCCEndPort(); usPort++)
		{
			hr = m_sockAccepter.Listen(usPort);
			if(SUCCEEDED(hr))
			{
				m_uLocalPort = usPort;

				hr = m_sockAccepter.Accept();
				_ASSERTE(SUCCEEDED(hr));

				if(FAILED(hr))
				{
					m_sockAccepter.Close();
				}

				break;
			}
		}

		_ASSERTE(SUCCEEDED(hr));
		if(SUCCEEDED(hr))
		{
			m_state = DCC_STATE_WAITING;
		}
		else
		{
			m_state = DCC_STATE_ERROR;
		}
		return SUCCEEDED(hr);
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// IDCCSession
/////////////////////////////////////////////////////////////////////////////
void DCCSend::Accept()
{
	_TRACE("DCCSend(0x%08X)::Accept()", this);
	_ASSERTE(m_state == DCC_STATE_REQUEST);

	if(m_state == DCC_STATE_REQUEST)
	{
		if(DoAcceptFileName())
		{
			m_hFile = CreateFile(m_sFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 
				NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if(m_hFile != INVALID_HANDLE_VALUE)
			{
				ULONG ulFileSize = ::GetFileSize(m_hFile, NULL);
				if(ulFileSize > 0)
				{
					int ret = MessageBox(GetActiveWindow(), _T("Would you like to resume a previous transfer?  Press NO to overwrite existing file."), APP_NAME, MB_YESNOCANCEL | MB_ICONQUESTION);
					if(ret == IDYES)
					{
						if(ulFileSize >= m_ulFileSize)
						{
							MessageBox(GetActiveWindow(), _T("Existing file is already same size or larger than sent file."), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
							m_state = DCC_STATE_CLOSED;
						}
						else
						{
							DWORD ret = SetFilePointer(m_hFile, ulFileSize, NULL, FILE_BEGIN);
							_ASSERTE(ret == ulFileSize);
							if(ret == 0xFFFFFFFF)
							{
								_TRACE("... SetFilePointer() FAILED: 0x%08X", GetLastError());
								m_state = DCC_STATE_CLOSED;
							}
							else
							{
								Session* pSession = m_pDCCHandler->GetSession();
								_ASSERTE(pSession != NULL);

								TCHAR buf[POCKETIRC_MAX_IRC_LINE_LEN];
								_sntprintf(buf, sizeof(buf), _T("RESUME \"\" %u %u"), GetRemotePort(), ulFileSize);
								buf[sizeof(buf) - 1] = 0;
								pSession->CTCP(GetRemoteUser(), _T("DCC"), buf);

								m_nTransferred = ulFileSize;
								m_state = DCC_STATE_WAITING;
							}
						}
					}
					else if(ret == IDNO)
					{
						Connect();
					}
					else if(ret == IDCANCEL)
					{
						m_state = DCC_STATE_CLOSED;
					}
				}
				else
				{
					Connect();
				}
			}
			else
			{
				_TRACE("... CreateFile() FAILED: 0x%08X", GetLastError());
				MessageBox(GetActiveWindow(), _T("Could not open file."), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
			}
		}
	}

	m_pDCCHandler->UpdateSession(this);

}

void DCCSend::Close()
{
	_TRACE("DCCSend(0x%08X)::Close()", this);

	if(m_sockAccepter.IsOpen())
	{
		m_sockAccepter.Close();
	}

	if(m_pTransport && m_pTransport->IsOpen())
	{
		m_pTransport->Close();

		CloseSendThread();

		delete m_pTransport;
		m_pTransport = NULL;
	}

	CloseFile();

	m_pDCCHandler->RemoveSession(this);
}

tstring DCCSend::GetRemoteUser()
{
	return GetPrefixNick(m_sRemoteUser);
}

tstring DCCSend::GetDescription()
{
	tstring str = m_bIncoming ? _T("RECV ") : _T("SEND ");
	str += m_sFileName;
	str += m_bIncoming ? _T(" From ") : _T(" To ");
	str += GetPrefixNick(m_sRemoteUser);

	return str;
}

bool DCCSend::Resume(ULONG ulSize)
{
	_TRACE("DCCSend(0x%08X)::Resume(%u)", this, ulSize);

	bool bOk = false;

	_TRACE("... Attempting to resume: %u", ulSize);
	if(ulSize < m_ulFileSize)
	{
		DWORD ret = SetFilePointer(m_hFile, ulSize, NULL, FILE_BEGIN);
		_ASSERTE(ret == ulSize);
		if(ret == 0xFFFFFFFF)
		{
			_TRACE("... SetFilePointer() FAILED: 0x%08X", GetLastError());
		}
		else
		{
			m_nTransferred = ulSize;
			bOk = true;
		}
	}
	else
	{
		_TRACE("... Nothing left to resume");
	}
	return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// ITransportReader
/////////////////////////////////////////////////////////////////////////////
void DCCSend::OnConnect(HRESULT hr, LPCTSTR pszError)
{
	_TRACE("DCCSend(0x%08X)::OnConnect(0x%08X, \"%s\")", this, hr, pszError);

	if(FAILED(hr))
	{
		m_state = DCC_STATE_ERROR;
	}
	else
	{
		if(hr == S_OK)
		{
			m_state = DCC_STATE_CONNECTED;
		}
		else
		{

		}
	}

	m_pDCCHandler->UpdateSession(this);

}

void DCCSend::OnError(HRESULT hr)
{
	_TRACE("DCCSend(0x%08X)::OnError(0x%08X)", this, hr);

	m_state = DCC_STATE_ERROR;
	m_pDCCHandler->UpdateSession(this);

	CloseFile();
}

void DCCSend::OnClose()
{
	_TRACE("DCCSend(0x%08X)::OnClose()", this);

	m_uLocalPort = 0;

	if(m_sockAccepter.IsOpen())
	{
		m_sockAccepter.Close();
	}

	if(m_nTransferred == m_ulFileSize)
	{
		m_state = DCC_STATE_COMPLETE;
	}
	else
	{
		m_state = DCC_STATE_CLOSED;
	}

	m_pDCCHandler->UpdateSession(this);

	CloseFile();
}

void DCCSend::OnRead(BYTE* pData, UINT nSize)
{
	_TRACE("DCCSend(0x%08X)::OnRead(0x%08X, %d)", this, pData, nSize);

	if(m_bIncoming)
	{
		_ASSERTE(m_hFile != INVALID_HANDLE_VALUE);
		if(m_hFile == INVALID_HANDLE_VALUE)
			return;

		UINT nWrite = nSize;
		if(m_nTransferred >= m_ulFileSize)
		{
			_TRACE("... File size already greater than size requested: %d > %d", m_nTransferred, m_ulFileSize);
			nWrite = 0;
		}
		else if(m_nTransferred + nWrite > m_ulFileSize)
		{
			_TRACE("... Packet exceeds original file size: %d of %d", m_nTransferred + nWrite, m_ulFileSize);
			nWrite = m_ulFileSize - m_nTransferred;
		}

		DWORD nWritten = 0;
		WriteFile(m_hFile, pData, nWrite, &nWritten, NULL);
		_TRACE("... Wrote: %d", nWritten);

		_ASSERTE(nWritten == nWrite);
		if(nWritten == nWrite)
		{
			m_nTransferred += nWritten;

			_TRACE("... Ack: %d", m_nTransferred);
			UINT nAck = htonl(m_nTransferred);
			m_pTransport->Write((BYTE*)&nAck, sizeof(nAck));

			if(m_nTransferred >= m_ulFileSize)
			{
				_TRACE("... Done.  Saved: %d", m_nTransferred);

				// That's it.
				CloseFile();
				m_pTransport->Close();
				delete m_pTransport;
				m_pTransport = NULL;

				m_state = DCC_STATE_COMPLETE;
				m_pDCCHandler->UpdateSession(this);
			}
		}
		else
		{
			_TRACE("... WriteFile() FAILED. Wrote: %d of %d", nWritten, nWrite);
			m_pTransport->Close();
			delete m_pTransport;
			m_pTransport = NULL;
		}
	}
	else
	{
		UINT nRead = 0;
		while(nSize - nRead >= sizeof(UINT))
		{
			UINT nAck = *(UINT*)(pData + nRead);
			_TRACE("... Got Ack: %d", nAck);

			nRead += sizeof(UINT);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//	ISocketAccepter Interface
/////////////////////////////////////////////////////////////////////////////

bool DCCSend::OnAccept(HRESULT hr, SocketTransport* pTransport, LPCTSTR psz)
{
	_TRACE("DCSend(0x%08X)::OnAccept(0x%08X, 0x%08X, \"%s\")", this, hr, pTransport, psz);
	// return true to stop accept thread loop

	if(FAILED(hr))
	{
		m_state = DCC_STATE_ERROR;
	}
	else
	{
		_ASSERTE(pTransport != NULL);
		_ASSERTE(m_pTransport == NULL);

		m_state = DCC_STATE_CONNECTED;

		m_pDCCHandler->UpdateSession(this);

		m_pTransport = pTransport;

		m_pTransport->SetReader(this);
		m_pTransport->Read();

		m_hSendThread = CreateThread(NULL, 0, &DCCSend::SendThreadStub, this, 0, NULL);
		_ASSERTE(m_hSendThread != NULL);
		if(m_hSendThread == NULL)
		{
			m_state = DCC_STATE_ERROR;
		}
		else
		{
			m_state = DCC_STATE_CONNECTED;
		}
	}

	m_pDCCHandler->UpdateSession(this);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Internal Utilities
/////////////////////////////////////////////////////////////////////////////

bool DCCSend::DoAcceptFileName()
{
	TCHAR szFileName[MAX_PATH];
	int nBuf = sizeof(szFileName)/sizeof(TCHAR);
	_tcsncpy(szFileName, m_sFileName.c_str(), nBuf);

	TCHAR szFilter[MAX_PATH] = _T("All Files (*.*)\0*.*\0");

	tstring sExt;
	TCHAR* pszExt = _tcsrchr(m_sFileName.c_str(), '.');
	if(pszExt != NULL && pszExt[0] != NULL)
	{
		sExt = pszExt + 1;

		wsprintf(szFilter, _T("%s Files (*.%s)|*.%s|All Files (*.*)|*.*|"), sExt.c_str(), sExt.c_str(), sExt.c_str());

		TCHAR* psz = szFilter;
		while(psz[0] != NULL)
		{
			if(psz[0] == '|')
				psz[0] = '\0';
			++psz;
		}
	}

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetActiveWindow();
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = szFileName;
	ofn.lpstrTitle = _T("Choose File Name");
	ofn.lpstrFilter = szFilter;
	ofn.Flags = OFN_PATHMUSTEXIST;

	if(GetSaveFileName(&ofn))
	{
		m_sFileName = szFileName;
		return true;
	}
	return false;
}

bool DCCSend::DoSendFileName()
{
	TCHAR szFileName[MAX_PATH] = _T("");
	int nBuf = sizeof(szFileName)/sizeof(TCHAR);

	TCHAR szFilter[MAX_PATH] = _T("All Files (*.*)\0*.*\0");

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetActiveWindow();
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = szFileName;
	ofn.lpstrTitle = _T("Choose File Name");
	ofn.lpstrFilter = szFilter;
	ofn.Flags = OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn))
	{
		m_hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, 
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(m_hFile != INVALID_HANDLE_VALUE)
		{
			m_sFileName = SafeDCCFileName(szFileName);
			m_ulFileSize = ::GetFileSize(m_hFile, NULL);
			return true;
		}
	}
	return false;
}

void DCCSend::CloseFile(bool bDelete)
{
	_TRACE("DCCSend(0x%08X)::CloseFile(%s)", this, bDelete ? _T("true") : _T("false"));

	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;

		if(bDelete)
		{
			DeleteFile(m_sFileName.c_str());
		}
	}
}

void DCCSend::CloseSendThread()
{
	_TRACE("DCCSend(0x%08X)::CloseSendThread()", this);

	if(!m_bIncoming && m_hSendThread)
	{
		WaitForSingleObject(m_hSendThread, INFINITE);
		CloseHandle(m_hSendThread);
		m_hSendThread = NULL;
	}	
}

DWORD CALLBACK DCCSend::SendThreadStub(LPVOID lpv)
{
	_TRACE("DCCSend()::SendThreadStub(0x%08X)", lpv);

	DCCSend* pSend = (DCCSend*)lpv;
	_ASSERTE(pSend != NULL);

	return pSend->SendThreadProc();

}

DWORD DCCSend::SendThreadProc()
{
	_TRACE("DCCSend(0x%08X)::SendThreadProc()", this);

	const UINT BLOCK_SIZE = 1024;
	BYTE buf[BLOCK_SIZE];

	HRESULT hr = S_OK;

	_TRACE("... Starting send at %u of %u", m_nTransferred, m_ulFileSize);

	while(m_nTransferred < m_ulFileSize)
	{
		DWORD nRead = 0;
		DWORD nNextBlock = BLOCK_SIZE;
		if(m_nTransferred + nNextBlock > m_ulFileSize)
		{
			nNextBlock = m_ulFileSize - m_nTransferred;
		}

		if(ReadFile(m_hFile, buf, nNextBlock, &nRead, NULL))
		{
			_TRACE("... Read: %d", nRead);

			if(nRead == 0)
			{
				_TRACE("... ReadFile returned EOF: Sent %u of %u", m_nTransferred, m_ulFileSize);
				break;
			}
			else
			{
				hr = m_pTransport->Write(buf, nRead);				
				if(SUCCEEDED(hr))
				{
					_TRACE("... Sent: %d", nRead);
					m_nTransferred += nRead;
				}
				else
				{
					_TRACE("... Send Failed: 0x%08X", hr);
					break;
				}
			}
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			_TRACE("... ReadFile Failed: 0x%08X", hr);

			break;
		}
	}
	return hr;
}

void DCCSend::Connect()
{
	_TRACE("DCCSend(0x%08X)::Connect()", this);

	m_pTransport = new SocketTransport();
	m_pTransport->SetNotify(ISocketNotify::Instance);
	m_pTransport->SetReader(this);

	HRESULT hr = m_pTransport->Connect(m_sRemoteHost, m_uRemotePort);
	_ASSERTE(SUCCEEDED(hr));

	if(SUCCEEDED(hr))
	{
		m_state = DCC_STATE_CONNECTING;
	}
	else
	{
		m_state = DCC_STATE_ERROR;
		delete m_pTransport;
		m_pTransport = NULL;

		CloseFile();
	}
}
