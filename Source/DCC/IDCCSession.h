#ifndef _IDCCSESSION_H_INCLUDED_
#define _IDCCSESSION_H_INCLUDED_

typedef enum _DCC_TYPE
{
	DCC_INVALID,
	DCC_CHAT,
	DCC_SEND,
} DCC_TYPE;

typedef enum _DCC_STATE
{
	DCC_STATE_ERROR,
	DCC_STATE_REQUEST,
	DCC_STATE_WAITING,
	DCC_STATE_CONNECTING,
	DCC_STATE_CONNECTED,
	DCC_STATE_COMPLETE,
	DCC_STATE_CLOSED,
} DCC_STATE;

class IDCCSession
{
public:
	virtual ~IDCCSession(){}

	virtual DCC_TYPE GetType() = 0;
	virtual DCC_STATE GetState() = 0;
	virtual bool IsIncoming() = 0;
	
	virtual void Accept() = 0;
	virtual void Close() = 0;

	virtual tstring GetRemoteUser() = 0;
	virtual tstring GetRemoteHost() = 0;
	virtual USHORT GetRemotePort() = 0;
	virtual USHORT GetLocalPort() = 0;

	virtual tstring GetDescription() = 0;
};

tstring GetDCCStateString(DCC_STATE state);

#endif//_IDCCSESSION_H_INCLUDED_
