
#ifndef _SERVER_
#define _SERVER_

#ifndef _DEBUG_
#define _DEBUG_ false
#endif


#include "NetworkAdapter.h"
#include "Observer.h"


class Server : public Observer {
private:
	NetworkAdapter* network;
	bool terminate;
	bool networkRunning;

	sf::Mutex mtx;
	
	

public:
	/* Const Vars */
	const std::string NAME = "Server";
	
	/* Constrution / Destruction */
	Server();
	~Server();

	/* Run / Stop */
	void Run();
	void Stop();
	
	
	
	/* Notification Handling */
	virtual void NetworkAdapter_ConnectionMessage(void * sender, EventArgs * e) override;
	virtual void NetworkAdapter_LobbyMessage(void* sender, EventArgs* e) override;
	virtual void NetworkAdapter_MatchMessage(void* sener, EventArgs* e) override;
	virtual void NetworkAdapter_ConnectionClosed(void* sender) override;

	


};



#endif
