#include "PocketIRC.h"

#include "IDCCSession.h"

tstring GetDCCStateString(DCC_STATE state)
{
	static TCHAR* STATES[] = 
	{
		_T("Error"),
		_T("Request"),
		_T("Waiting"),
		_T("Connecting"),
		_T("Connected"),
		_T("Complete"),
		_T("Closed"),
	};

	if(state < DCC_STATE_ERROR || state >= sizeof(STATES)/sizeof(*STATES))
	{
		_ASSERTE(FALSE);
		return _T("Unknown");
	}
	return STATES[state];
}
