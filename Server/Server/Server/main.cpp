#ifndef _DEBUG_
#define _DEBUG_ true
#endif

#include "Server.h"
#include <string>
#include <map>

using namespace std;
using namespace sf;


int main(void) {

	const string NAME = "main";
	bool terminate = false;

	map<unsigned short, pair<IpAddress, unsigned short>> users;
	
	
	Time ellapsed = seconds(0);
	Time sleepDelay = microseconds(50);
	Clock clock;
	Mutex mtx;


	string userCmd = "";

	if (_DEBUG_) {
		mtx.lock();
		cout << "[" << NAME << "] Starting Server.\n";
		mtx.unlock();
	}

	Server server;
	server.Run();
	
	while (true) {

		// DEBUG
		cin >> userCmd;
		if (userCmd == "quit") {
			break;
		}
		sleep(sleepDelay);


		//// delay then look again
		//if (!network.HasIncomingPacket()) {
		//	sleep(sleepDelay);
		//	continue;
		//}

		//// read message
		//np = network.FetchPacket();
		//if (np == 0 || np->packet == 0) {
		//	sleep(sleepDelay);
		//	continue;
		//}

		//// evaluate code
		//code = 0;
		//p = np->packet;
		//*p >> code;
		//switch (code) {
		//case NetworkAdapter::Code::CONNECT:
		//	np->packet->clear();
		//	*(np->packet) << code << currMaxSessionId;
		//	np->port = 4321;
		//	users[currMaxSessionId] = make_pair(np->Ip, np->port);
		//	currMaxSessionId++;
		//	network.SendPacket(np);

		//	if (_DEBUG_) {
		//		mtx.lock();
		//		cout << "Accepted Client: SessionID(" << (currMaxSessionId-1) << "), IP(" << np->Ip << "), Port(" << np->port << ")\n";
		//		mtx.unlock();
		//	}

		//	break;

		//case NetworkAdapter::Code::DISCONNECT:

		//	break;

		//case NetworkAdapter::Code::PLAER_DATA:
		//	*p >> sessionId >> r >> g >> b >> dx >> dy;
		//	p->clear();
		//	p = new Packet;
		//	*p << code << sessionId << r << g << b << dx << dy;
		//	np->packet = p;

		//	for (pair< unsigned short, pair<IpAddress, unsigned short >> kvp : users) {
		//		//if (sessionId == kvp.first) {
		//		//	continue;
		//		//}

		//		np->Ip = kvp.second.first;
		//		np->port = kvp.second.second;
		//		network.SendPacket(np);
		//	}
		//	break;

		//}

	}


	server.Stop();

	cin >> userCmd;

	return 0;

}