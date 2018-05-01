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

			BotScript(int aBotId, std::string aBotName, int aVersionId, uint64_t viewerKey, std::string aCode)
				: bot_id(aBotId), version_id(aVersionId), viewer_key(viewerKey)
				, bot_name(aBotName), code(aCode)
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
			virtual void ReportBotKilled(long victim_id, long version_id, long start_frame, long end_frame, long killer_id, double final_mass) = 0;
			virtual void DisableBotVersion(long version_id, std::string errorMessage) = 0;

			virtual void SetupLiveStats(long user_id) = 0;
			virtual void UpdateLiveStats(long user_id, long current_frame, double mass,
					double natural_food_consumed, double carrison_food_consumed,
					double hunted_food_consumed) = 0;
			virtual void RemoveLiveStats(long user_id) = 0;
	};

	class MysqlDatabase : public IDatabase
	{
		public:
			void Connect(std::string host, std::string username, std::string password, std::string database);
			std::unique_ptr<BotScript> GetBotData(int bot_id) override;
			std::vector<int> GetActiveBotIds() override;
			std::vector<Command> GetActiveCommands() override;
			void SetCommandCompleted(long commandId, bool result, std::string resultMsg) override;
			void ReportBotKilled(long victim_id, long version_id, long start_frame, long end_frame, long killer_id, double final_mass) override;
			void DisableBotVersion(long version_id, std::string errorMessage) override;

			void SetupLiveStats(long user_id) override;
			void UpdateLiveStats(long user_id, long current_frame, double mass,
					double natural_food_consumed, double carrison_food_consumed,
					double hunted_food_consumed) override;
			void RemoveLiveStats(long user_id) override;

		private:
			enum {
				IDX_BOTSCRIPT_BOT_ID = 1,
				IDX_BOTSCRIPT_BOT_NAME = 2,
				IDX_BOTSCRIPT_VERSION_ID = 3,
				IDX_BOTSCRIPT_CODE = 4,
				IDX_BOTSCRIPT_VIEWER_KEY = 5,
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
			std::unique_ptr<sql::PreparedStatement> _setupLiveStatsStmt;
			std::unique_ptr<sql::PreparedStatement> _updateLiveStatsStmt;
			std::unique_ptr<sql::PreparedStatement> _removeLiveStatsStmt;
			std::unique_ptr<sql::PreparedStatement> makePreparedStatement(std::string sql);
	};
}
