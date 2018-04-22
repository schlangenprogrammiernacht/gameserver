#include "Database.h"
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace db;

void MysqlDatabase::Connect(std::string host, std::string username, std::string password, std::string database)
{
	_driver = get_driver_instance();
	_connection = std::unique_ptr<sql::Connection>(_driver->connect(host, username, password));
	_connection->setSchema(database);

	_getBotScriptStmt = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"SELECT "
			"	s.id AS snake_id, "
			"	s.name AS snake_name, "
			"	sv.id AS version_id, "
			"	sv.code AS code "
			"FROM core_snakeversion sv "
			"LEFT JOIN core_snake s ON (sv.snake_id=s.id) "
			"WHERE sv.id=(SELECT MAX(id) FROM core_snakeversion WHERE snake_id=?);"
		)
	);

	_getAllBotScriptsStmt = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"SELECT "
			"	s.id AS snake_id, "
			"	s.name AS snake_name, "
			"	sv.id AS version_id, "
			"	sv.code AS code "
			"FROM core_snakeversion sv "
			"LEFT JOIN core_snake s ON (sv.snake_id=s.id) "
			"WHERE sv.id=(SELECT MAX(id) FROM core_snakeversion WHERE snake_id=s.id);"
		)
	);

	_getActiveBotIdsStmt = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"SELECT s.id AS snake_id "
			"FROM core_snakeversion sv "
			"LEFT JOIN core_snake s ON (sv.snake_id=s.id) "
			"WHERE sv.id=(SELECT MAX(id) FROM core_snakeversion WHERE snake_id=s.id);"
		)
	);
}

std::vector<BotScript> MysqlDatabase::GetBotScript(int bot_id)
{
	_getBotScriptStmt->setInt(1, bot_id);
	return GetScripts(_getBotScriptStmt.get());
}

std::vector<BotScript> MysqlDatabase::GetBotScripts()
{
	return GetScripts(_getAllBotScriptsStmt.get());
}

std::vector<int> MysqlDatabase::GetActiveBotIds()
{
	std::unique_ptr<sql::ResultSet> res(_getAllBotScriptsStmt->executeQuery());
	std::vector<int> retval;
	while (res->next())
	{
		retval.push_back(res->getInt(1));
	}
	return retval;
}

std::vector<BotScript> MysqlDatabase::GetScripts(sql::PreparedStatement *stmt)
{
	std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
	std::vector<BotScript> retval;
	while (res->next())
	{
		retval.emplace_back(
			res->getInt(IDX_BOTSCRIPT_BOT_ID),
			res->getString(IDX_BOTSCRIPT_BOT_NAME),
			res->getInt(IDX_BOTSCRIPT_VERSION_ID),
			res->getString(IDX_BOTSCRIPT_CODE)
		);
	}
	return retval;
}

