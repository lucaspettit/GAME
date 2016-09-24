/** Observer
  * Base Observer class for all files which need to listen for events will inharit
  * Has static 'do-nothing' functions for all events.
  * Child Observer class will need to override functions and add handlers.
  */


#ifndef _OBSERVER_
#define _OBSERVER_

#ifndef _DEBUG_
#define _DEBUG_ false
#endif

#include <string>

class EventArgs {
public:
	const std::string NAME = "EventArgs";
	const int Id;
	const std::string Message;

	EventArgs(int id, std::string message);
	std::string ToString();
};


class Observer {

public:
	virtual void NetworkAdapter_ConnectionMessage(void* sender, EventArgs* e) { delete(e); };
	virtual void NetworkAdapter_LobbyMessage(void* sender, EventArgs* e) { delete(e); };
	virtual void NetworkAdapter_MatchMessage(void* sender, EventArgs* e) { delete(e); };
	virtual void NetworkAdapter_ConnectionOpend(void* sender) {};
	virtual void NetworkAdapter_ConnectionClosed(void* sender) {};

};


#endif 