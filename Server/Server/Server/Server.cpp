#include "Server.h"

using namespace std;

Server::Server() {
	terminate = true;

	network = new NetworkAdapter(NetworkAdapter::NetworkType::SERVER);
	network->AddHandler(this, NetworkAdapter::Event::CONNECTION_MESSAGE);
	network->AddHandler(this, NetworkAdapter::Event::LOBBY_MESSAGE);
	network->AddHandler(this, NetworkAdapter::Event::MATCH_MESSAGE);
}

Server::~Server() {
	Stop();
}

void Server::Run() {
	terminate = false;
	networkRunning = true;
	network->Connect();
}

void Server::Stop() {
	if (networkRunning) {
		network->Disconnect();
		networkRunning = false;
	}
}

void Server::NetworkAdapter_ConnectionMessage(void * sender, EventArgs * e)
{
	mtx.lock();
	NetworkAdapterEventArgs* args = (NetworkAdapterEventArgs*)e;
	cout << "new connection request: IP=" << args->Ip << ", port=" << args->port << endl;
	mtx.unlock();
	delete(e);
}

void Server::NetworkAdapter_LobbyMessage(void * sender, EventArgs * e)
{
	mtx.lock();
	NetworkAdapterEventArgs* args = (NetworkAdapterEventArgs*)e;
	cout << "new lobby message: IP=" << args->Ip << ", port=" << args->port << endl;
	mtx.unlock();
	delete(e);
}

void Server::NetworkAdapter_MatchMessage(void * sender, EventArgs * e)
{
	mtx.lock();
	NetworkAdapterEventArgs* args = (NetworkAdapterEventArgs*)e;
	cout << "new match player update: IP=" << args->Ip << ", port=" << args->port << endl;
	mtx.unlock();
	delete(e);
}

void Server::NetworkAdapter_ConnectionClosed(void * sender)
{
	Stop();
}


