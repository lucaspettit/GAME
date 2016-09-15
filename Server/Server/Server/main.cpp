#ifndef _DEBUG_
#define _DEBUG_ true
#endif

#include "NetworkAdapter.h"
#include <string>
#include <map>

using namespace std;
using namespace sf;


int main(void) {

	bool terminate = false;

	map<unsigned short, pair<IpAddress, unsigned short>> users;
	unsigned short currMaxSessionId = 1;
	unsigned short sessionId, userPort, code;
	IpAddress userAddress;

	
	
	Time ellapsed = seconds(0);
	Time sleepDelay = microseconds(50);
	Clock clock;
	Mutex mtx;

	NPacket* np;
	Packet* p;

	// player data
	Uint8 r = 0, g = 0, b = 0;
	float dx = 0, dy = 0;



	string userCmd = "";

	if (_DEBUG_) {
		mtx.lock();
		cout << "Starting Server.\n";
		mtx.unlock();
	}

	NetworkAdapter network(NetworkAdapter::NetworkType::SERVER);

	// Start listening for clients
	Thread network_thread(&NetworkAdapter::Connect, &network);
	network_thread.launch();

	
	while (true) {

		// delay then look again
		if (!network.HasIncomingPacket()) {
			sleep(sleepDelay);
			continue;
		}

		// read message
		np = network.FetchPacket();
		if (np == 0 || np->packet == 0) {
			sleep(sleepDelay);
			continue;
		}

		// evaluate code
		code = 0;
		p = np->packet;
		*p >> code;
		switch (code) {
		case NetworkAdapter::Code::CONNECT:
			np->packet->clear();
			*(np->packet) << code << currMaxSessionId;
			np->port = 4321;
			users[currMaxSessionId] = make_pair(np->Ip, np->port);
			currMaxSessionId++;
			network.SendPacket(np);

			if (_DEBUG_) {
				mtx.lock();
				cout << "Accepted Client: SessionID(" << (currMaxSessionId-1) << "), IP(" << np->Ip << "), Port(" << np->port << ")\n";
				mtx.unlock();
			}

			break;

		case NetworkAdapter::Code::DISCONNECT:

			break;

		case NetworkAdapter::Code::PLAER_DATA:
			*p >> sessionId >> r >> g >> b >> dx >> dy;
			p->clear();
			p = new Packet;
			*p << code << sessionId << r << g << b << dx << dy;
			np->packet = p;

			for (pair< unsigned short, pair<IpAddress, unsigned short >> kvp : users) {
				//if (sessionId == kvp.first) {
				//	continue;
				//}

				np->Ip = kvp.second.first;
				np->port = kvp.second.second;
				network.SendPacket(np);
			}
			break;

		}

	}


	network.Disconnect();
	network_thread.wait();

	return 0;

}