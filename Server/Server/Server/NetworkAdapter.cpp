#include "NetworkAdapter.h"

using namespace std;
using namespace sf;

NetworkAdapterEventArgs::NetworkAdapterEventArgs(int id, string msg) : EventArgs(id, msg) {}

NetworkAdapterEventArgs::~NetworkAdapterEventArgs() {
	delete(packet);
}

/* Constructor */
NetworkAdapter::NetworkAdapter(bool is_server) : isServer(is_server) {
	if (isServer) {
		inPort = 1234;
		outPort = 4321;
		
		id_counter = 0;
		available_ids.push(id_counter);
	}
	else {
		inPort = 4666;
		outPort = 1234;

		id_counter = 0;
	}
}

/* Destructor */
NetworkAdapter::~NetworkAdapter() {
	Disconnect();
}


/* Connect */
void NetworkAdapter::Connect() {

	if (_DEBUG_) {
		mtx.lock();
		cout << "[" << NAME << "] Connecting to network." << endl;
		mtx.unlock();
	}

	//// Create UDP socket
	//UdpSocket udpSocket;
	//// Create ThreadData Object
	//MatchThreadData* threadArgs = new MatchThreadData;
	//threadArgs->npInQueue = &q_match_IN;
	//threadArgs->npOutQueue = &q_match_OUT;
	//threadArgs->inPort = inPort;
	//threadArgs->outPort = outPort;
	//td.push_back(threadArgs);


	// Start Listening for Incomming Connections
	nctd.caller = this;
	nctd.q_OUT = &q_connection_message_OUT;
	nctd.mtx = &mtx;
	nctd.selector = &tcp_client_selector;
	nctd.pending_clients = &tcp_clients;
	nctd.terminate = false;

	// Launch Listen thread
	thread ncListenThread(TCP_NewConnection_Listen, &nctd);
	ncListenThread.detach();
	connected = true;


	//// Test Events // WORKS!
	//NetworkAdapterEventArgs* openConnEvent = new NetworkAdapterEventArgs(NetworkAdapterEvent::CONNECTION_OPEN, "");
	//NetworkAdapterEventArgs* closeConnEvent = new NetworkAdapterEventArgs(NetworkAdapterEvent::CONNECTION_CLOSED, "");
	//NetworkAdapterEventArgs* connMsgEvent = new NetworkAdapterEventArgs(NetworkAdapterEvent::CONNECTION_MESSAGE, "");
	//NetworkAdapterEventArgs* lobbyMsgEvent = new NetworkAdapterEventArgs(NetworkAdapterEvent::LOBBY_MESSAGE, "");
	//NetworkAdapterEventArgs* matchMsgEvent = new NetworkAdapterEventArgs(NetworkAdapterEvent::MATCH_MESSAGE, "");

	//RaiseEvent(NetworkAdapterEvent::CONNECTION_OPEN, openConnEvent);
	//RaiseEvent(NetworkAdapterEvent::CONNECTION_CLOSED, closeConnEvent);
	//RaiseEvent(NetworkAdapterEvent::CONNECTION_MESSAGE, connMsgEvent);
	//RaiseEvent(NetworkAdapterEvent::LOBBY_MESSAGE, lobbyMsgEvent);
	//RaiseEvent(NetworkAdapterEvent::MATCH_MESSAGE, matchMsgEvent);

	//// Create Threads
	//Thread listenThread(&UDP_Listen, threadArgs);
	////Thread speakThread(&UDP_Speak, threadArgs);

	//// Launch Threads
	//listenThread.launch();
	////speakThread.launch();

	//// Wait Disconnection Signal
	//while (!terminate) {
	//	sf::sleep(seconds(1));
	//}

	//// Therminate Threads
	//listenThread.terminate();

	//// Free All Data
	//NPacket* np;
	//while (q_match_IN.size() > 0) {
	//	np = q_match_IN.front();
	//	delete(np->packet);
	//	delete(np);
	//	q_match_IN.pop();
	//}

}

/* Disconnect */
void NetworkAdapter::Disconnect() {
	if (connected) {
		if (_DEBUG_) {
			mtx.lock();
			cout << "[" << NAME << "] Disconnecting from network." << endl;
			mtx.unlock();
		}

		terminate = true;
		nctd.terminate = true;
		connected = false;

		RaiseEvent(Event::CONNECTION_CLOSED, NULL);
	}
}


/* AddHandler */
void NetworkAdapter::AddHandler(Observer* observer, int eventId) {

	switch (eventId) {
	case Event::CONNECTION_MESSAGE:
		ConnectionMessageHandlers.push_back(observer);
		break;

	case Event::LOBBY_MESSAGE:
		LobbyMessageHandlers.push_back(observer);
		break;

	case Event::MATCH_MESSAGE:
		MatchMessageHanlders.push_back(observer);
		break;

	case Event::CONNECTION_OPEN:
		ConnectionOpenedHandlers.push_back(observer);
		break;

	case Event::CONNECTION_CLOSED:
		ConnectionClosedHandlers.push_back(observer);
		break;
	}

}

/* RaiseEvent */
void NetworkAdapter::RaiseEvent(int eventId, NetworkAdapterEventArgs* np) {

	switch (eventId) {
	case Event::CONNECTION_MESSAGE:
		for (Observer* observer : ConnectionMessageHandlers) {
			observer->NetworkAdapter_ConnectionMessage(this, np);
		}
		break;

	case Event::LOBBY_MESSAGE:
		for (Observer* observer : LobbyMessageHandlers) {
			observer->NetworkAdapter_LobbyMessage(this, np);
		}
		break;

	case Event::MATCH_MESSAGE:
		for (Observer* observer : MatchMessageHanlders) {
			observer->NetworkAdapter_MatchMessage(this, np);
		}
		break;

	case Event::CONNECTION_OPEN:
		for (Observer* observer : ConnectionOpenedHandlers) {
			observer->NetworkAdapter_ConnectionOpend(this);
		}
		break;

	case Event::CONNECTION_CLOSED:
		for (Observer* observer : ConnectionClosedHandlers) {
			observer->NetworkAdapter_ConnectionClosed(this);
		}
	}

}

/* Remove Handler */
void NetworkAdapter::RemoveHandler(Observer* observer, int eventId) {
	/*switch (eventId) {
	case NetworkAdapterEvent::CONNECTION_MESSAGE:
		for (int i = 0; i < ConnectionMessageHandlers.size(); i++) {
			if (&ConnectionMessageHandlers[i] == &observer) {
				ConnectionMessageHandlers.erase[i];
				break;
			}
		}
		break;

	case NetworkAdapterEvent::LOBBY_MESSAGE:
		for (int i = 0; i < LobbyMessageHandlers.size(); i++) {
			if (&LobbyMessageHandlers[i] == &observer) {
				LobbyMessageHandlers.erase[i];
				break;
			}
		}
		break;

	case NetworkAdapterEvent::MATCH_MESSAGE:
		for (int i = 0; i < MatchMessageHanlders.size(); i++) {
			if (&MatchMessageHanlders[i] == &observer) {
				MatchMessageHanlders.erase[i];
				break;
			}
		}
		break;

	case NetworkAdapterEvent::CONNECTION_OPEN:
		for (int i = 0; i < ConnectionOpenedHandlers.size(); i++) {
			if (&ConnectionOpenedHandlers[i] == &observer) {
				ConnectionOpenedHandlers.erase[i];
				break;
			}
		}
		break;

	case NetworkAdapterEvent::CONNECTION_CLOSED:
		for (int i = 0; i < ConnectionClosedHandlers.size(); i++) {
			if (&ConnectionClosedHandlers[i] == &observer) {
				ConnectionClosedHandlers.erase[i];
				break;
			}
		}
		break;

	}*/
}

/* Send Message */
void NetworkAdapter::SendMessage(unsigned short sessionId, short messageTypeId, sf::Packet packet) {

	switch (messageTypeId) {
	case Message::MATCH:
		if (match_clients.find(sessionId) != match_clients.end()) {

		}
		break;

	case Message::CHAT:
		if (chat_clients.find(sessionId) != chat_clients.end()) {

		}
		break;

	case Message::CONNECTION:
	case Message::LOBBY:
		if (tcp_clients.find(sessionId) != tcp_clients.end()) {

			TcpSocket* client = tcp_clients.at(sessionId);
			client->send(packet);
		}
		break;
	}
}


/* Disconnect Client */
void NetworkAdapter::RemoveClient(unsigned short sessionId) {
	if (tcp_clients.find(sessionId) != tcp_clients.end()) {
		mtx.lock();
		tcp_clients.erase(sessionId);
		mtx.unlock();
	}
	if (match_clients.find(sessionId) != match_clients.end()) {
		mtx.lock();
		match_clients.erase(sessionId);
		mtx.unlock();
	}
	if (chat_clients.find(sessionId) != chat_clients.end()) {
		mtx.lock();
		chat_clients.erase(sessionId);
		mtx.unlock();
	}
	available_ids.push(sessionId);
}

/* Match Join/Quit */
void NetworkAdapter::JoinMatchListener(unsigned short sessionId) {
	// Add client to match
}
void NetworkAdapter::QuitMatchListener(unsigned short sessionId) {
	// Add client to match
}

/* Chat Join/Quit */
void NetworkAdapter::JoinChatListener(unsigned short sessionId) {
	// Add client to match
}
void NetworkAdapter::QuitChatListener(unsigned short sessionId) {
	// Add client to match
}





