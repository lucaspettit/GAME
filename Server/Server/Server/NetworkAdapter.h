#pragma once

#ifndef _DEBUG_
#define _DEBUG_ true
#endif


#include <SFML/Network.hpp>
#include <vector>
#include <queue>
#include <iostream>
#include <string>
#include <exception>

/* NPacket */
class NPacket {
public:
	unsigned short port;
	sf::IpAddress Ip;
	sf::Packet* packet;
};

/* ThreadData */
class ThreadData {
public:

	bool terminate = false;
	unsigned short inPort;
	unsigned short outPort;
	sf::IpAddress serverIp = "127.0.0.1";
	std::queue<NPacket*>* npInQueue;
	std::queue<NPacket*>* npOutQueue;
};

/* NetworkAdapter */
class NetworkAdapter { 
private:
	sf::Mutex mtx;
	const bool isServer;
	unsigned short inPort, outPort;
	bool terminate = false;

	std::queue<NPacket*> np_inq, np_outq;
	std::vector<ThreadData*> td;
	sf::IpAddress serverIp = "127.0.0.1";

	sf::UdpSocket outSocket;

	

	/* UDP_Speak */
	static void NetworkAdapter::UDP_Speak(ThreadData* args) {

		// Make Vars
		sf::UdpSocket socket;
		sf::Packet packet;
		NPacket* npacket;
		sf::Mutex mtx;

		// Write Packets To Client If Needed
		while (!args->terminate) {
			try {
				if (args->npOutQueue->size() > 0) {
					mtx.lock();
					npacket = args->npOutQueue->front();
					socket.send(*npacket->packet, npacket->Ip, npacket->port);
					args->npOutQueue->pop();
					mtx.unlock();

					delete(npacket->packet);
					delete(npacket);
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

	/* UDP_Listen */
	static void NetworkAdapter::UDP_Listen(ThreadData* args) {

		// Make Vars
		sf::UdpSocket socket;
		NPacket* npacket;
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
				npacket = new NPacket;
				npacket->Ip = senderIp;
				npacket->port = senderPort;
				npacket->packet = &packet;
				mtx.lock();
				args->npInQueue->push(npacket);
				mtx.unlock();

				//if (_DEBUG_) {
				//	unsigned short code = 0;
				//	unsigned short clientId = 0;
				//	sf::Uint8 r = 0, g = 0, b = 0;
				//	float x = 0, y = 0;
				//	packet >> code >> clientId >> r >> g >> b >> x >> y;
				//	packet << code << clientId << r << g << b << x << y;
				//	mtx.lock();
				//	std::cout << "Packet received:[" << code << "] ClientID(" << clientId;
				//	std::cout << "), RGB=(" << std::to_string(r);
				//	std::cout << ", " << std::to_string(g);
				//	std::cout << ", " << std::to_string(b);
				//	std::cout << "), Position(" << std::to_string(x);
				//	std::cout << ", " << std::to_string(y) << ")\n";
				//	mtx.unlock();
				//}
			}
			catch (std::exception ex) {
				mtx.lock();
				std::cout << "Exception occered! " << ex.what() << std::endl;
				mtx.unlock();
			}
		}
	}

public:

	const enum NetworkType {
		CLIENT = false,
		SERVER = true
	};

	const enum Code {
		DNS = 0,
		CONNECT = 1,
		DISCONNECT = 2,

		PLAER_DATA = 10
	};

	NetworkAdapter(bool is_server);

	void Connect();
	void Disconnect();

	bool HasIncomingPacket();
	NPacket* FetchPacket();
	void SendPacket(sf::Packet* p);
	void SendPacket(NPacket* np);
};


