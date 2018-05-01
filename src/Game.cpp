#include <iostream>

#include "config.h"
#include "Environment.h"
#include "debug_funcs.h"
#include "MsgPackUpdateTracker.h"
#include "Game.h"
#include "LuaBot.h"

Game::Game()
{
	m_field = std::make_unique<Field>(
		config::FIELD_SIZE_X, config::FIELD_SIZE_Y,
		config::FIELD_STATIC_FOOD,
		std::make_unique<MsgPackUpdateTracker>()
	);

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
			long killer_id = (killer==nullptr) ? -1 : killer->getDatabaseId();
			m_database->ReportBotKilled(
				victim->getDatabaseId(),
				victim->getDatabaseVersionId(),
				victim->getStartFrame(),
				m_field->getCurrentFrame(),
				killer_id,
				victim->getSnake()->getMass()
			);

			m_database->RemoveLiveStats(
				victim->getDatabaseId()
			);

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
	initTracker.worldState(*m_field);
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

	m_field->decayFood();
	m_field->consumeFood();
	m_field->removeFood();
	m_field->moveAllBots();

	if(++m_streamStatsUpdateCounter >= STREAM_STATS_UPDATE_INTERVAL) {
		m_field->sendStatsToStream();
		m_streamStatsUpdateCounter = 0;
	}

	if(++m_dbStatsUpdateCounter >= DB_STATS_UPDATE_INTERVAL) {
		updateStatsInDB();
		m_dbStatsUpdateCounter = 0;
	}

	m_field->processLog();
	m_field->tick();

	// send differential update to all connected clients
	std::string update = m_field->getUpdateTracker().serialize();
	server.Broadcast(update);

	if (++m_dbQueryCounter >= DB_QUERY_INTERVAL)
	{
		queryDB();
		m_dbQueryCounter = 0;
	}

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

	for (auto id: m_database->GetActiveBotIds())
	{
		createBot(id);
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
	auto active_ids = m_database->GetActiveBotIds();
	for (auto id: active_ids)
	{
		if (m_field->getBotByDatabaseId(id) == nullptr)
		{
			createBot(id);
		}
	}

	std::vector<std::shared_ptr<Bot>> kill_bots;
	for (auto& bot: m_field->getBots())
	{
		if (std::find(active_ids.begin(), active_ids.end(), bot->getDatabaseId()) == active_ids.end())
		{
			kill_bots.push_back(bot);
		}
	}

	for (auto& bot: kill_bots)
	{
		m_field->killBot(bot, bot); // suicide!
	}

	for (auto& cmd: m_database->GetActiveCommands())
	{
		if (cmd.command == db::Command::CMD_KILL)
		{
			auto bot = m_field->getBotByDatabaseId(static_cast<int>(cmd.bot_id));
			if (bot != nullptr)
			{
				m_field->killBot(bot, bot); // suicide!
				m_database->SetCommandCompleted(cmd.id, true, "killed");
			}
			else
			{
				m_database->SetCommandCompleted(cmd.id, false, "bot not known / not active");
			}
		}
		else
		{
			m_database->SetCommandCompleted(cmd.id, false, "command not known");
		}
	}
}

void Game::updateStatsInDB(void)
{
	for(auto &b: m_field->getBots()) {
		m_database->UpdateLiveStats(b->getDatabaseId(),
				m_field->getCurrentFrame(),
				b->getSnake()->getMass(),
				b->getConsumedNaturalFood(),
				b->getConsumedFoodHuntedByOthers(),
				b->getConsumedFoodHuntedBySelf());
	}
}

void Game::createBot(int bot_id)
{
	auto data = m_database->GetBotData(bot_id);
	if (data == nullptr)
	{
		return;
	}

	std::string initErrorMessage;
	auto newBot = m_field->newBot(std::move(data), initErrorMessage);
	if (!initErrorMessage.empty())
	{
		m_database->DisableBotVersion(newBot->getDatabaseVersionId(), initErrorMessage);
		// TODO save error message, maybe lock version in inactive state
	} else {
		m_database->SetupLiveStats(newBot->getDatabaseId());
	}
}
