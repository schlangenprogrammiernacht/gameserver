/*
 * Schlangenprogrammiernacht: A programming game for GPN18.
 * Copyright (C) 2018  bytewerk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>

#include "Game.h"
#include "config.h"
#include "Environment.h"
#include "debug_funcs.h"
#include "MsgPackUpdateTracker.h"
#include "Stopwatch.h"

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
				victim->getSnake()->getMass(),
				victim->getConsumedNaturalFood(),
				victim->getConsumedFoodHuntedByOthers(),
				victim->getConsumedFoodHuntedBySelf()
			);

			// victim will be respawned on next database query
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

	socket.SetWriteBlocking(false);

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

void Game::ProcessOneFrame()
{
	// do all the game logic here and send updates to clients
	auto frame = m_field->getCurrentFrame();

	Stopwatch swProcessFrame("ProcessFrame");

	Stopwatch swDecayFood("DecayFood");
	m_field->decayFood();
	swDecayFood.Stop();

	Stopwatch swConsumeFood("ConsumeFood");
	m_field->consumeFood();
	swConsumeFood.Stop();

	Stopwatch swRemoveFood("RemoveFood");
	m_field->removeFood();
	swRemoveFood.Stop();

	Stopwatch swMoveAllBots("MoveAllBots");
	m_field->moveAllBots();
	swMoveAllBots.Stop();

	Stopwatch swProcessStats("ProcessStats");
	if(++m_streamStatsUpdateCounter >= STREAM_STATS_UPDATE_INTERVAL) {
		m_field->sendStatsToStream();
		m_streamStatsUpdateCounter = 0;
	}
	swProcessStats.Stop();

	Stopwatch swProcessLog("ProcessLog");
	m_field->processLog();
	swProcessLog.Stop();

	Stopwatch swProcessTick("ProcessTick");
	m_field->tick();
	swProcessTick.Stop();

	Stopwatch swLimbo("Limbo");
	m_field->updateLimbo();
	swLimbo.Stop();

	Stopwatch swSendUpdate("SendUpdate");
	// send differential update to all connected clients
	std::string update = m_field->getUpdateTracker().serialize();
	server.Broadcast(update);
	swSendUpdate.Stop();

	Stopwatch swQueryDB("QueryDB");
	if (++m_dbQueryCounter >= DB_QUERY_INTERVAL)
	{
		queryDB();
		m_dbQueryCounter = 0;
	}
	swQueryDB.Stop();

	swProcessFrame.Stop();

#if DEBUG_TIMINGS
	std::cout << std::endl;
	std::cout << "Frame " << frame << " timings: " << std::endl;
	swDecayFood.Print();
	swConsumeFood.Print();
	swRemoveFood.Print();
	swMoveAllBots.Print();
	swProcessStats.Print();
	swProcessLog.Print();
	swProcessTick.Print();
	swSendUpdate.Print();
	swLimbo.Print();
	swQueryDB.Print();
	swProcessFrame.Print();
	std::cout << std::endl;
#endif
}

int Game::Main()
{
	// set up umask so we can create shared files for the bots
	umask(0000);

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

	while(true)
	{
		ProcessOneFrame();
		server.Poll(0);
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
		if (!m_field->isDatabaseIdActive(id))
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

void Game::createBot(int bot_id)
{
	auto data = m_database->GetBotData(bot_id);
	if (data == nullptr)
	{
		return;
	}

	if(data->compile_state != "successful") {
		return;
	}

	std::string initErrorMessage;
	auto newBot = m_field->newBot(std::move(data), initErrorMessage);
	if (!initErrorMessage.empty())
	{
		m_database->DisableBotVersion(newBot->getDatabaseVersionId(), initErrorMessage);
		// TODO save error message, maybe lock version in inactive state
	}
}
