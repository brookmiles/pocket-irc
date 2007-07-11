
DCC Window Can:
	Accept DCC Chat or Send Requests
	Initiate DCC Send to User
	Initiate DCC Chat to User
	Close Any DCC Connection/Request
	Close All DCC Connections/Requests

DCC List Entry Consists of:

o Type: CHAT, SEND, etc...
o Direction: To or From
o Connection Info: User, IP, Port
o State, can be one of:
	REQUEST (received from user)
	WAITING (for reply to sent request)
	CONNECTING
	CONNECTED
	COMPLETE
	CLOSED
	ERROR
o Status: free form error/information message

Tree structure