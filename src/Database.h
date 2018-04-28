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
			std::string bot_name;
			std::string code;

			BotScript(int aBotId, std::string aBotName, int aVersionId, std::string aCode)
				: bot_id(aBotId), version_id(aVersionId)
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
			virtual std::vector<BotScript> GetBotScript(int bot_id) = 0;
			virtual std::vector<BotScript> GetBotScripts() = 0;
			virtual std::vector<int> GetActiveBotIds() = 0;
			virtual std::vector<Command> GetActiveCommands() = 0;
			virtual void SetCommandCompleted(long commandId, bool result, std::string resultMsg) = 0;
			virtual void ReportBotKilled(long victim_id, long version_id, long start_frame, long end_frame, long killer_id, double final_mass) = 0;
	};

	class MysqlDatabase : public IDatabase
	{
		public:
			void Connect(std::string host, std::string username, std::string password, std::string database);
			std::vector<BotScript> GetBotScript(int bot_id) override;
			std::vector<BotScript> GetBotScripts() override;
			std::vector<int> GetActiveBotIds() override;
			std::vector<Command> GetActiveCommands() override;
			void SetCommandCompleted(long commandId, bool result, std::string resultMsg) override;
			void ReportBotKilled(long victim_id, long version_id, long start_frame, long end_frame, long killer_id, double final_mass) override;

		private:
			enum {
				IDX_BOTSCRIPT_BOT_ID = 1,
				IDX_BOTSCRIPT_BOT_NAME = 2,
				IDX_BOTSCRIPT_VERSION_ID = 3,
				IDX_BOTSCRIPT_CODE = 4
			};

			sql::Driver *_driver = nullptr;
			std::unique_ptr<sql::Connection> _connection;
			std::unique_ptr<sql::PreparedStatement> _getBotScriptStmt;
			std::unique_ptr<sql::PreparedStatement> _getAllBotScriptsStmt;
			std::unique_ptr<sql::PreparedStatement> _getActiveBotIdsStmt;
			std::unique_ptr<sql::PreparedStatement> _getActiveCommandsStmt;
			std::unique_ptr<sql::PreparedStatement> _commandCompletedStmt;
			std::unique_ptr<sql::PreparedStatement> _reportBotKilledStmt;

			std::vector<BotScript> GetScripts(sql::PreparedStatement *stmt);
			std::unique_ptr<sql::PreparedStatement> makePreparedStatement(std::string sql);
	};
}
