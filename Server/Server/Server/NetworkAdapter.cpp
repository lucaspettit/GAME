#include "NetworkAdapter.h"

using namespace std;
using namespace sf;

/* Constructor */
NetworkAdapter::NetworkAdapter(bool is_server) : isServer(is_server) {
	if (isServer) {
		inPort = 1234;
		outPort = 4321;
	}
	else {
		inPort = 4666;
		outPort = 1234;
	}
}

/* Connect */
void NetworkAdapter::Connect() {

	if (_DEBUG_) {
		mtx.lock();
		cout << "Connecting to network." << endl;
		mtx.unlock();
	}

	// Create UDP socket
	UdpSocket udpSocket;
	// Create ThreadData Object
	ThreadData* threadArgs = new ThreadData;
	threadArgs->npInQueue = &np_inq;
	threadArgs->npOutQueue = &np_outq;
	threadArgs->inPort = inPort;
	threadArgs->outPort = outPort;
	td.push_back(threadArgs);

	// Create Threads
	Thread listenThread(&UDP_Listen, threadArgs);
	//Thread speakThread(&UDP_Speak, threadArgs);

	// Launch Threads
	listenThread.launch();
	//speakThread.launch();

	// Wait Disconnection Signal
	while (!terminate) {
		sf::sleep(seconds(1));
	}

	// Therminate Threads
	listenThread.terminate();

	// Free All Data
	NPacket* np;
	while (np_inq.size() > 0) {
		np = np_inq.front();
		delete(np->packet);
		delete(np);
		np_inq.pop();
	}

}

/* Disconnect */
void NetworkAdapter::Disconnect() {

	if (_DEBUG_) {
		mtx.lock();
		cout << "Disconnecting from network." << endl;
		mtx.unlock();
	}

	terminate = true;
}

/* HasIncommingPacket */
bool NetworkAdapter::HasIncomingPacket() {
	return (np_inq.size() > 0) ? true : false;
}

/* FetchPacket */
NPacket* NetworkAdapter::FetchPacket() {
	if (np_inq.size() > 0) {
		NPacket* npacket = np_inq.front();
		np_inq.pop();
		return npacket;
	}
	else {
		return 0;
	}
}

/* SendPacket */
void NetworkAdapter::SendPacket(Packet* p) {
	
	outSocket.send(*p, serverIp, outPort);
	
}

void NetworkAdapter::SendPacket(NPacket* np) {

	outSocket.send(*np->packet, np->Ip, np->port);
}






