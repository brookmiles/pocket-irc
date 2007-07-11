#ifndef _FORMAT_H_INCLUDED_
#define _FORMAT_H_INCLUDED_

#include "StringT.h"

class Format
{
public:
	Format();
	Format(Format& from);
	~Format();

	const Format& operator=(Format& from);

	const String& GetEventName() const { return m_sName; }
	void SetEventName(const String& sName) { m_sName = sName; }

	int GetEventID() const { return m_idEvent; } 
	void SetEventID(int idEvent) { m_idEvent = idEvent; }

	bool IsIncoming() const { return m_bIncoming; }
	void SetIncoming(bool bIncoming) { m_bIncoming = bIncoming; } 

	const String& GetFormat() const { return m_sFormat; }
	void SetFormat(const String& sFormat) { m_sFormat = sFormat; }

	bool IsEnabled() const { return m_bEnabled; }
	void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

private:
	String m_sName;
	int m_idEvent;
	String m_sFormat;
	bool m_bIncoming;
	bool m_bEnabled;
};

#endif//_FORMAT_H_INCLUDED_
