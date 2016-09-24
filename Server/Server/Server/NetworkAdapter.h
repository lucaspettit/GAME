#pragma once

#ifndef _DEBUG_
#define _DEBUG_ true
#endif

#include "Observer.h"
#include <SFML/Network.hpp>
#include <thread>
#include <time.h>
#include <vector>
#include <queue>
#include <iostream>
#include <string>
#include <exception>

class NetworkAdapter;
class NetworkAdapterEventArgs;

class MatchThreadData;
class SessionData;

/* NetworkAdapterEventArgs */
class NetworkAdapterEventArgs  : public EventArgs {
public:
	unsigned short port = 0, sessionId = 0;
	sf::IpAddress Ip;
	sf::Packet* packet;

	NetworkAdapterEventArgs(int id, std::string msg);
	~NetworkAdapterEventArgs();
};

/* NewConnectThreadData*/
class NewConnectThreadData {
public:
	NetworkAdapter* caller;
	bool terminate = false;
	sf::Mutex* mtx;
	std::queue<NetworkAdapterEventArgs*>* q_OUT;
	sf::SocketSelector* selector;
	std::map<unsigned short, sf::TcpSocket*>* pending_clients;
	std::queue<unsigned short> available_ids;
	unsigned short id_counter;
};

/* MatchThreadData */
class MatchThreadData {
public:

	NetworkAdapter* caller;
	bool terminate = false;
	unsigned short inPort;
	unsigned short outPort;
	sf::IpAddress serverIp = "127.0.0.1";
	std::queue<NetworkAdapterEventArgs*>* npOutQueue;
	std::vector<SessionData*> clientVector;


};


/* NetworkAdapter */
class NetworkAdapter { 
public:
	const enum Event {
		CONNECTION_OPEN,
		CONNECTION_CLOSED,
		CONNECTION_MESSAGE,
		LOBBY_MESSAGE,
		MATCH_MESSAGE
	};

	const enum Message {
		CONNECTION,
		LOBBY,
		MATCH,
		CHAT
	};

	 const enum Port {
		NEW_CONNECT = 1234,
		MATCH_FIRST = 3321,
		MATCH_FINAL = 4321
	};

	const enum NetworkType {
		CLIENT = false,
		SERVER = true
	};

	const enum Code {
		DNS = 0,
		CONNECT = 1,
		DISCONNECT = 2,

		PLAER_DATA = 100
	};

private:
	const std::string NAME = "NetworkAdapter";
	sf::Mutex mtx;
	const bool isServer;
	unsigned short inPort, outPort;
	bool terminate = false;
	bool connected = false;

	// Thread Data - Track to terminate threads
	std::vector<MatchThreadData*> thread_data;
	std::vector<SessionData*> clients;

	std::map<unsigned short, sf::TcpSocket*> tcp_clients;
	std::map<unsigned short, std::pair< sf::IpAddress, unsigned short>> match_clients, chat_clients;
	sf::SocketSelector tcp_client_selector;

	// SessionID Vars
	unsigned short id_counter = 0;
	std::queue<unsigned short> available_ids;


	// Out Message Queues
	std::queue< NetworkAdapterEventArgs* > q_connection_message_OUT;
	std::queue< NetworkAdapterEventArgs* > q_lobby_message_OUT;
	std::queue< NetworkAdapterEventArgs* > q_match_message_OUT;

	std::vector<MatchThreadData*> td;
	NewConnectThreadData nctd;
	sf::IpAddress serverIp = "127.0.0.1";

	sf::UdpSocket outSocket;

	// Observer/Subject Pattern Vars
	std::vector < Observer* > ConnectionMessageHandlers;
	std::vector < Observer* > LobbyMessageHandlers;
	std::vector < Observer* > MatchMessageHanlders;
	std::vector < Observer* > ConnectionOpenedHandlers;
	std::vector < Observer* > ConnectionClosedHandlers;

	/* RaiseEvent */
	void RaiseEvent(int eventId, NetworkAdapterEventArgs* args); 


	/*_____ STATIC FUNCTIONS _______*/
	/* TCP_NewConnection_Listen */
	static void NetworkAdapter::TCP_NewConnection_Listen(NewConnectThreadData* args) {

		/*____________ Make Vars____________*/
		// Socket/Listeners/Ports
		sf::TcpListener listener;
		sf::TcpSocket* client = new sf::TcpSocket;
		sf::TcpSocket* out_client;
		
		// Data Storage/Tracking/Control
		unsigned short tmp = 0;

		// Debug
		const std::string NAME = "NetworkAdapter";

		// Debug output
		if (_DEBUG_) {
			args->mtx->lock();
			std::cout << "[" << NAME << "] Starting NewConnection Listen thread : Port " << std::to_string(NetworkAdapter::Port::NEW_CONNECT) << std::endl;
			args->mtx->unlock();
		}

		// Launch IO Thread
		std::thread ioThread(TCP_NewConnection_IO, args);
		ioThread.detach();


		// Begin Listening And Check For Error
		while (!args->terminate) {

			// Listen For New Client
			switch (listener.listen((unsigned short)NetworkAdapter::Port::NEW_CONNECT)) {
			case sf::Socket::Done:
				break;

			default:
				sf::sleep(sf::milliseconds(200));
				continue;
				break;
			}


			// Make New Client Socket If Needed
			if (client->getLocalPort() != 0) {
				client = new sf::TcpSocket;
			}

			// Accept New Client
			if (listener.accept(*client) != sf::Socket::Done) {
				if (_DEBUG_) {
					args->mtx->lock();
					std::cout << "[" << NAME << "] Unable to accept new client. Total client count = " << std::to_string(args->pending_clients->size()) << std::endl;
					args->mtx->unlock();
				}
			}

			// Add Client To Map And Selector
			tmp = args->available_ids.front();
			args->available_ids.pop();
			if (tmp == args->id_counter) {
				args->id_counter += 1;
				args->available_ids.push(args->id_counter);
			}

			args->pending_clients->insert({ tmp, client });
			args->mtx->lock();
			args->selector->add(*client);
			args->mtx->unlock();
		}

		if (_DEBUG_) {
			args->mtx->lock();
			std::cout << "[" << NAME << "] Closing NewConnection Listener\n";
			args->mtx->unlock();
		}

		// Close Listener
		listener.close();
	}

	/* TCP_NewConnection_IO */
	static void NetworkAdapter::TCP_NewConnection_IO(NewConnectThreadData* args) {

		/*____________ Make Vars____________*/
		const std::string NAME = "NetworkAdapter";
		sf::TcpSocket* client;
		sf::Packet* packet;
		NetworkAdapterEventArgs* eventArgs;
		
		if (_DEBUG_) {
			args->mtx->lock();
			std::cout << "[" << NAME << "] Starting NewConnection IO thread\n";
			args->mtx->unlock();
		}

		while (!args->terminate) {

			// Check For Incomming Messages
			for (std::pair<unsigned short, sf::TcpSocket*> kvp : *(args->pending_clients)) {
				if (args->selector->isReady(*(kvp.second))) {
					packet = new sf::Packet;
					kvp.second->receive(*packet);

					if (_DEBUG_) {
						args->mtx->lock();
						std::cout << "[" << NAME << "] Raising ConnectionRequest event\n";
						args->mtx->unlock();
					}

					// build NetworkAdapterEventArgs and raise event
					eventArgs = new NetworkAdapterEventArgs(Event::CONNECTION_MESSAGE, "");
					eventArgs->Ip = kvp.second->getRemoteAddress();
					eventArgs->port = kvp.second->getRemotePort();
					eventArgs->packet = packet;
					args->caller->RaiseEvent(Event::CONNECTION_MESSAGE, eventArgs);
				}
			}

			sf::sleep(sf::milliseconds(200));
		}

		if (_DEBUG_) {
			args->mtx->lock();
			std::cout << "[" << NAME << "] Closing NewConnection IO thread\n";
			args->mtx->unlock();
		}
	}

	/* UDP_Listen */
	static void NetworkAdapter::UDP_Listen(MatchThreadData* args) {

		// Make Vars
		sf::UdpSocket socket;
		NetworkAdapterEventArgs* eventArgs;
		sf::Packet packet;
		sf::IpAddress senderIp;
		unsigned short senderPort;
		sf::Mutex mtx;

		socket.bind(args->inPort);

		if (_DEBUG_) {
			mtx.lock();
			std::cout << "Bound to port " << args->inPort << std::endl;
			mtx.unlock();
		}

		// Begin Listening Loop
		while (!args->terminate) {

			try {
				if (socket.receive(packet, senderIp, senderPort) != sf::Socket::Done) {
					return;
				}

				// build packet and push to queue
				eventArgs = new NetworkAdapterEventArgs(Event::MATCH_MESSAGE, "");
				eventArgs->Ip = senderIp;
				eventArgs->port = senderPort;
				eventArgs->packet = &packet;

				args->caller->RaiseEvent(Event::MATCH_MESSAGE, eventArgs);
			}
			catch (std::exception ex) {
				mtx.lock();
				std::cout << "Exception occered! " << ex.what() << std::endl;
				mtx.unlock();
			}
		}
	}

	/* UDP_Speak */
	static void NetworkAdapter::UDP_Speak(MatchThreadData* args) {

		// Make Vars
		sf::UdpSocket socket;
		sf::Packet packet;
		NetworkAdapterEventArgs* eventArgs;
		sf::Mutex mtx;

		// Write Packets To Client If Needed
		while (!args->terminate) {
			try {
				if (args->npOutQueue->size() > 0) {
					mtx.lock();
					eventArgs = args->npOutQueue->front();
					socket.send(*eventArgs->packet, eventArgs->Ip, eventArgs->port);
					args->npOutQueue->pop();
					mtx.unlock();

					delete(eventArgs->packet);
					delete(eventArgs);
				}
				else {
					sf::sleep(sf::milliseconds(5));
				}
			}
			catch (std::exception ex) {
				mtx.lock();
				std::cout << "Exception occured! " << ex.what() << std::endl;
				mtx.unlock();
			}
		}
	}

public:

	/* Construction/Destruction */
	NetworkAdapter(bool is_server);
	~NetworkAdapter();

	/* Connect/Disconnect */
	void Connect();
	void Disconnect();


	/* Handlers */
	void AddHandler(Observer* observer, int eventId);
	void RemoveHandler(Observer* observer, int eventId);


	/* Send Messages */
	void SendMessage(unsigned short sessionId, short messageTypeId, sf::Packet packet);


	/* Other */
	void RemoveClient(unsigned short sessionId);
	

	/** Not sure if network should do any internal checking for permissions to speak on ports
	  * Pros: Network adapter can guarentee that if a Match or Chat Message is raised, that that client is
	  *	      in a match or chat session. (makes programming easier for developer).
	  * Cons: User (developer) is expected to remember to start and stop these sessions.
	  */
	void JoinMatchListener(unsigned short sessionId);
	void QuitMatchListener(unsigned short sessionId);

	void JoinChatListener(unsigned short sessionId);
	void QuitChatListener(unsigned short sessionId);
};



