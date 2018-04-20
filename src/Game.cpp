#include <iostream>

#include "MsgPackUpdateTracker.h"

#include "config.h"
#include "debug_funcs.h"

#include "Game.h"

Game::Game()
{
	m_updateTracker = std::make_shared<MsgPackUpdateTracker>();
	m_field = std::make_shared<Field>(
			config::FIELD_SIZE_X, config::FIELD_SIZE_Y,
			config::FIELD_STATIC_FOOD,
			m_updateTracker);

	for(int i = 0; i < 20; i++) {
		m_field->newBot("testBot");
	}

	server.AddConnectionEstablishedListener(
		[this](TcpSocket& socket)
		{
			return OnConnectionEstablished(socket);
		}
	);

	server.AddConnectionClosedListener(
		[this](TcpSocket& socket)
		{
			return OnConnectionClosed(socket);
		}
	);

	server.AddDataAvailableListener(
		[this](TcpSocket& socket)
		{
			return OnDataAvailable(socket);
		}
	);

	server.AddTimerListener(
		[this](int, uint64_t expirationCount)
		{
			OnTimerInterval();
			return true;
		}
	);
}

bool Game::OnConnectionEstablished(TcpSocket &socket)
{
	std::cerr << "connection established to " << socket.GetPeer() << std::endl;

	// send initial state
	MsgPackUpdateTracker initTracker;
	initTracker.gameInfo();
	initTracker.worldState(m_field);
	socket.Write(initTracker.serialize());

	return true;
}

bool Game::OnConnectionClosed(TcpSocket &socket)
{
	std::cerr << "connection to " << socket.GetPeer() << " closed." << std::endl;

	return true;
}

bool Game::OnDataAvailable(TcpSocket &socket)
{
	char data[1024];
	ssize_t count = socket.Read(data, sizeof(data));
	if(count > 0) {
		// return to sender
		socket.Write(data, count, false);
	}
	return true;
}

bool Game::OnTimerInterval()
{
	// do all the game logic here and send updates to clients

	static uint32_t frameNumber = 0;
	frameNumber++;
	//std::cout << "Frame number #" << frameNumber << std::endl;

	m_field->decayFood();
	m_field->consumeFood();
	m_field->removeFood();

	m_field->moveAllBots();

	m_updateTracker->tick(frameNumber);

	// send differential update to all connected clients
	std::string update = m_updateTracker->serialize();
	server.Broadcast(update);

	//std::cout << hexdump(update) << std::endl;

	//m_field->debugVisualization();

	return true;
}

int Game::Main()
{
	if(!server.Listen(9010)) {
		return -1;
	}

	server.AddIntervalTimer(16666); // 60 fps
	//server.AddIntervalTimer(50000); // 20 fps
	//server.AddIntervalTimer(1000000); // 1 fps

	while(true) {
		server.Poll(1000);
	}
}
