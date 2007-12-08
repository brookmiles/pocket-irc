#ifndef _IDCCCHAT_H_INCLUDED_
#define _IDCCCHAT_H_INCLUDED_

class IDCCChat
{
public:
	virtual void Say(const tstring& str) = 0;
	virtual void Act(const tstring& str) = 0;
	virtual void CloseChat() = 0;

	virtual tstring GetRemoteUser() = 0;
};

#endif//_IDCCCHAT_H_INCLUDED_
