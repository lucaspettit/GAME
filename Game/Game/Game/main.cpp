
#ifndef _DEBUG_
#define _DEBUG_ true
#endif

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <random>
#include <iostream>
#include <vector>

#include "NetworkAdapter.h"

using namespace sf;
using namespace std;


int main(void){

	sf::Time TargetDelay = sf::milliseconds(16); 
	sf::Time ellapsed = sf::seconds(0);
	sf::Time delay = sf::seconds(0);
	int frames = 0;

	int window_w = 1024;
	int window_h = 732;
	bool flicker = false;
	float speed = 5.f;
	float dx = 0;
	float dy = 0;

	unsigned short sessionId = 0, code = 0;
	unsigned short senderId = 0;
	vector<CircleShape> other_players;
	Packet* packet;

	Clock clock;
	Mutex mtx;

	if (_DEBUG_) {
		cout << "____ GAME ATTRIBUTES ____" << endl;
		cout << "TargetDelay = " << TargetDelay.asSeconds() << endl;
		cout << "window = " << window_w << ", " << window_h << endl;
		cout << "speed = " << speed << endl;
	}

	// make circle and properties
	CircleShape player = CircleShape(20.f);
	Color playerColor(Color::Blue);
	player.setFillColor(playerColor);
	player.move((window_w - (int)20.f) / 2, (window_h - (int)20.f) / 2);

	// make complicated and efficient random 
	uniform_int_distribution<int> randomColorRange(0, 255); // this is light-weight and can be used and discarded whenver
	random_device rd; // super heavey, try and use only once if possible
	mt19937 randomNumbers(rd()); // Probably pretty heavy too. some kinda sweet random algorithm 

	Event e;
	
	// Create network and start listening for server
	NetworkAdapter network(NetworkAdapter::NetworkType::CLIENT);
	Thread network_thread(&NetworkAdapter::Connect, &network);
	network_thread.launch();

	// Request SessionId
	code = NetworkAdapter::Code::CONNECT;
	packet = new Packet;
	*packet << code;


	if (_DEBUG_) {
		mtx.lock();
		cout << "Requesting connection from server.\n";
		mtx.unlock();
	}

	clock.restart();
	while (ellapsed < seconds(30)) {
		
		// exit when paket comes in
		if (network.HasIncomingPacket()) {
			packet = network.FetchPacket();
			if (packet == 0) {
				if (_DEBUG_) {
					mtx.lock();
					cout << "Received null packet.\n";
					mtx.unlock();
				}
			}
			else {
				break;
			}
		}
		

		if (_DEBUG_) {
			mtx.lock();
			cout << ".";
			mtx.unlock();
		}

		network.SendPacket(packet);
		ellapsed += clock.getElapsedTime();

		sleep(seconds(1));
	}

	if (ellapsed > seconds(30)) {
		if (_DEBUG_) {
			mtx.lock();
			cout << "Server timeout.\n";
			mtx.unlock();
		}
		network.Disconnect();
		return -1;
	}
	
	if (_DEBUG_) {
		mtx.lock();
		cout << endl;
		mtx.unlock();
	}
	ellapsed = seconds(0);

	// receive session id
	*packet >> code;
	if (code != NetworkAdapter::Code::CONNECT) {
		network.Disconnect();

		if (_DEBUG_)
			cout << "Server returned with invalid code: " << code << endl;

		return 1;
	}
	
	*packet >> sessionId;
	//free(packet);

	if (_DEBUG_) {
		mtx.lock();
		cout << "Connected to server: SessionID = " << sessionId << endl;
		mtx.unlock();
	}

	// make window and set properties
	RenderWindow window(VideoMode(window_w, window_h), "SFML Game");
	window.setActive(false);
	window.setKeyRepeatEnabled(true); // This raises an event for every frame that a key is held down

	// Start Game Loop
	while (window.isOpen()) {
		
		// reset clock
		clock.restart();

		// poll events
		while (window.pollEvent(e)) {
			switch (e.type) {
			case Event::Closed:
				network.Disconnect();
				window.close();
				break;

			case Event::EventType::KeyPressed:

				switch (e.key.code) {
				case Keyboard::W:
					dy -= speed;
					break;

				case Keyboard::S:
					dy += speed;
					break;

				case Keyboard::A:
					dx -= speed;
					break;

				case Keyboard::D:
					dx += speed;
					break;

				case Keyboard::Space:
					flicker = (flicker) ? false : true;
				}
				break;

			case Event::EventType::KeyReleased:
				switch (e.key.code) {
				case Keyboard::W:
					dy = 0;
					break;

				case Keyboard::S:
					dy = 0;
					break;

				case Keyboard::A:
					dx = 0;
					break;

				case Keyboard::D:
					dx = 0;
					break;
				}
			}
		}

		// read incoming messages
		while (true) {
			packet = network.FetchPacket();
			if (!packet) 
				break;

			*packet >> code;
			if (code == 0) 
				break;
			
			*packet >> senderId;
			switch (code) {
			case NetworkAdapter::Code::PLAER_DATA:
				CircleShape other = CircleShape(20.f);
				Uint8 r, g, b;
				float x, y;
				*packet >> r >> g >> b >> x >> y;
				Color c(r, g, b);
				other.setFillColor(c);
				other.move(x, y);

				other_players.push_back(other);
				//delete(packet);
				break;
			}
		}

		// flicker 
		if (flicker) {
			playerColor.r = randomColorRange(randomNumbers);
			playerColor.g = randomColorRange(randomNumbers);
			playerColor.b = randomColorRange(randomNumbers);
			player.setFillColor(playerColor);
		}

		// cap speed
		if (dx < -speed)
			dx = -speed;
		else if (dx > speed)
			dx = speed;

		if (dy < -speed)
			dy = -speed;
		else if (dy > speed)
			dy = speed;

		if (_DEBUG_) {
			ellapsed += clock.getElapsedTime();
			frames += 1;

			if (ellapsed >= seconds(1)) {
				mtx.lock();
				cout << "FPS = " << to_string(frames / ellapsed.asSeconds()) << endl;
				mtx.unlock();
				frames = 0;
				ellapsed = seconds(0);
			}
		}

		packet = new Packet;
		code = NetworkAdapter::Code::PLAER_DATA;
		*packet << code << sessionId << playerColor.r << playerColor.g << playerColor.b << dx << dy;
		network.SendPacket(packet);

		player.move(dx, dy);

		// draw
		window.clear(Color::White);

		for (int i = other_players.size() - 1; i >= 0; i--) {
			window.draw(other_players[i]);
			other_players.pop_back();
		}

		window.draw(player);
		window.display();
		

		delay = TargetDelay - clock.getElapsedTime();
		sleep(delay);
	}
	return 0;
}