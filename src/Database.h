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

	class IDatabase
	{
		public:
			virtual ~IDatabase() = default;
			virtual std::vector<BotScript> GetBotScript(int bot_id) = 0;
			virtual std::vector<BotScript> GetBotScripts() = 0;
			virtual std::vector<int> GetActiveBotIds() = 0;
	};

	class MysqlDatabase : public IDatabase
	{
		public:
			void Connect(std::string host, std::string username, std::string password, std::string database);
			std::vector<BotScript> GetBotScript(int bot_id) override;
			std::vector<BotScript> GetBotScripts() override;
			std::vector<int> GetActiveBotIds() override;

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

			std::vector<BotScript> GetScripts(sql::PreparedStatement *stmt);
	};
}
