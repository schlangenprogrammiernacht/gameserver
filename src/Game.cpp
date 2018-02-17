#include <iostream>

#include "MsgPackUpdateTracker.h"

#include "debug_funcs.h"

#include "Game.h"

Game::Game()
{
	m_updateTracker = std::make_shared<MsgPackUpdateTracker>();
	m_field = std::make_shared<Field>(60, 20, 30, m_updateTracker);

	m_field->newBot("testBot");

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
			for (uint64_t i=0; i<expirationCount; i++)
			{
				OnTimerInterval();
			}
			return true;
		}
	);
}

bool Game::OnConnectionEstablished(TcpSocket &socket)
{
	std::cerr << "connection established to " << socket.GetPeer() << std::endl;

	socket.Write("Hello World!\n");

	// TODO: send GameInfoMessage
	// TODO: send WorldUpdateMessage
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
	std::cout << "Frame number #" << frameNumber++ << std::endl;

	m_field->updateFood();
	m_field->consumeFood();

	m_field->moveAllBots();

	std::string update = m_updateTracker->serialize();
	std::cout << hexdump(update) << std::endl;

	m_field->debugVisualization();

	return true;
}

int Game::Main()
{
	if(!server.Listen(9010)) {
		return -1;
	}

	//server.AddIntervalTimer(16666); // 60 fps
	server.AddIntervalTimer(1000000); // 1 fps

	while(true) {
		server.Poll(1000);
	}
}
