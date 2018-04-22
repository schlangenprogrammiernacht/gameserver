#include <iostream>

#include "config.h"
#include "Environment.h"
#include "debug_funcs.h"
#include "MsgPackUpdateTracker.h"
#include "Game.h"
#include "LuaBot.h"

Game::Game()
{
	m_updateTracker = std::make_shared<MsgPackUpdateTracker>();
	m_field = std::make_shared<Field>(
			config::FIELD_SIZE_X, config::FIELD_SIZE_Y,
			config::FIELD_STATIC_FOOD,
			m_updateTracker);

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

	m_field->addBotKilledCallback(
		[this](std::shared_ptr<Bot> victim, std::shared_ptr<Bot> killer)
		{
			createBot(victim->getDatabaseId());
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

	++m_currentFrame;

	if (++m_dbQueryCounter >= DB_QUERY_INTERVAL)
	{
		queryDB();
		m_dbQueryCounter = 0;
	}

	//std::cout << "Frame number #" << frameNumber << std::endl;

	m_field->decayFood();
	m_field->consumeFood();
	m_field->removeFood();

	m_field->moveAllBots();

	m_updateTracker->tick(m_currentFrame);

	// send differential update to all connected clients
	std::string update = m_updateTracker->serialize();
	server.Broadcast(update);

	//std::cout << hexdump(update) << std::endl;

	//m_field->debugVisualization();

	return true;
}

int Game::Main()
{
	if (!server.Listen(9010))
	{
		return -1;
	}

	if (!connectDB())
	{
		return -2;
	}

	for (auto& script: m_database->GetBotScripts())
	{
		createBot(script.bot_id);
	}

	server.AddIntervalTimer(16666); // 60 fps
	//server.AddIntervalTimer(50000); // 20 fps
	//server.AddIntervalTimer(1000000); // 1 fps

	while(true) {
		server.Poll(1000);
	}
}

bool Game::connectDB()
{
	auto db = std::make_unique<db::MysqlDatabase>();
	db->Connect(
		Environment::GetDefault(Environment::ENV_MYSQL_HOST, Environment::ENV_MYSQL_HOST_DEFAULT),
		Environment::GetDefault(Environment::ENV_MYSQL_USER, Environment::ENV_MYSQL_USER_DEFAULT),
		Environment::GetDefault(Environment::ENV_MYSQL_PASSWORD, Environment::ENV_MYSQL_PASSWORD_DEFAULT),
		Environment::GetDefault(Environment::ENV_MYSQL_DB, Environment::ENV_MYSQL_DB_DEFAULT)
	);
	m_database = std::move(db);
	return true;
}

void Game::queryDB()
{
	for (auto& v: m_database->GetBotScripts())
	{
		if (m_field->getBotByDatabaseId(v.bot_id) == nullptr)
		{
			createBot(v.bot_id);
		}
	}

	// TODO kill bots that are not in the database (any more)?
	// TODO query commands
}

void Game::createBot(int bot_id)
{
	auto res = m_database->GetBotScript(bot_id);
	if (res.size() != 0)
	{
		auto luaBot = std::make_unique<LuaBot>();
		luaBot->init(res[0].code);
		m_field->newBot(res[0].bot_id, res[0].bot_name, std::move(luaBot));
	}
}

