#pragma once

#include <memory>

#include <TcpServer/TcpServer.h>

#include "UpdateTracker.h"
#include "Field.h"
#include "Database.h"

class Game
{
	private:
		TcpServer server;

		std::shared_ptr<UpdateTracker> m_updateTracker;
		std::shared_ptr<Field> m_field;
		std::unique_ptr<db::IDatabase> m_database;
		bool connectDB();

	public:
		Game();

		bool OnConnectionEstablished(TcpSocket &socket);
		bool OnConnectionClosed(TcpSocket &socket);
		bool OnDataAvailable(TcpSocket &socket);
		bool OnTimerInterval();

		int Main();
};
