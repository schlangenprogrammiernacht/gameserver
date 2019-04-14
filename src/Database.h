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

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>

namespace db
{
	class BotScript
	{
		public:
			int bot_id = -1;
			int version_id = -1;
			uint64_t viewer_key = 0;
			std::string bot_name;
			std::string code;
			std::string compile_state;

			BotScript(int aBotId, std::string aBotName, int aVersionId, uint64_t viewerKey, std::string aCode, std::string compileState)
				: bot_id(aBotId), version_id(aVersionId), viewer_key(viewerKey)
				, bot_name(aBotName), code(aCode), compile_state(compileState)
			{}
	};

	class Command
	{
		public:
			static constexpr const char* CMD_KILL = "kill";

			long id = -1;
			long bot_id = -1;
			std::string command;

			Command(long commandId, long botId, std::string cmd)
				: id(commandId), bot_id(botId), command(cmd)
			{}
	};

	class IDatabase
	{
		public:
			virtual ~IDatabase() = default;
			virtual std::unique_ptr<BotScript> GetBotData(int bot_id) = 0;
			virtual std::vector<int> GetActiveBotIds() = 0;
			virtual std::vector<Command> GetActiveCommands() = 0;
			virtual void SetCommandCompleted(long commandId, bool result, std::string resultMsg) = 0;
			virtual void ReportBotKilled(long victim_id, long version_id, long start_frame, long end_frame, long killer_id, double maximum_mass, double final_mass, double natural_food_consumed, double carrison_food_consumed, double hunted_food_consumed) = 0;
			virtual void DisableBotVersion(long version_id, std::string errorMessage) = 0;
			virtual void SetBotToCrashedState(long version_id) = 0;
			virtual void UpdateLiveStats(double fps, uint64_t current_frame, uint32_t running_bots, uint32_t start_queue_len, uint32_t stop_queue_len, double living_mass, double dead_mass) = 0;
	};

	class MysqlDatabase : public IDatabase
	{
		public:
			void Connect(std::string host, std::string username, std::string password, std::string database);
			std::unique_ptr<BotScript> GetBotData(int bot_id) override;
			std::vector<int> GetActiveBotIds() override;
			std::vector<Command> GetActiveCommands() override;
			void SetCommandCompleted(long commandId, bool result, std::string resultMsg) override;
			void ReportBotKilled(long victim_id, long version_id, long start_frame, long end_frame, long killer_id, double maximum_mass, double final_mass, double natural_food_consumed, double carrison_food_consumed, double hunted_food_consumed) override;
			void DisableBotVersion(long version_id, std::string errorMessage) override;
			void SetBotToCrashedState(long version_id) override;
			void UpdateLiveStats(double fps, uint64_t current_frame, uint32_t running_bots, uint32_t start_queue_len, uint32_t stop_queue_len, double living_mass, double dead_mass) override;

		private:
			enum {
				IDX_BOTSCRIPT_BOT_ID = 1,
				IDX_BOTSCRIPT_BOT_NAME = 2,
				IDX_BOTSCRIPT_VERSION_ID = 3,
				IDX_BOTSCRIPT_CODE = 4,
				IDX_BOTSCRIPT_COMPILE_STATE = 5,
				IDX_BOTSCRIPT_VIEWER_KEY = 6,
			};

			sql::Driver *_driver = nullptr;
			std::unique_ptr<sql::Connection> _connection;
			std::unique_ptr<sql::PreparedStatement> _getBotDataStmt;
			std::unique_ptr<sql::PreparedStatement> _getActiveBotIdsStmt;
			std::unique_ptr<sql::PreparedStatement> _getActiveCommandsStmt;
			std::unique_ptr<sql::PreparedStatement> _commandCompletedStmt;
			std::unique_ptr<sql::PreparedStatement> _reportBotKilledStmt;
			std::unique_ptr<sql::PreparedStatement> _disableBotVersionStmt;
			std::unique_ptr<sql::PreparedStatement> _saveBotVersionErrorMessageStmt;
			std::unique_ptr<sql::PreparedStatement> _setBotToCrashedStateStmt;
			std::unique_ptr<sql::PreparedStatement> _updateLiveStatsStmt;
			std::unique_ptr<sql::PreparedStatement> makePreparedStatement(std::string sql);
	};
}
