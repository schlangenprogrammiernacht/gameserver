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

	_getBotScriptStmt = makePreparedStatement(
		"SELECT u.id, u.username, sv.id, sv.code "
		"FROM core_activesnake a "
		"LEFT JOIN core_snakeversion sv ON (sv.id=a.version_id) "
		"LEFT JOIN auth_user u ON (u.id=a.user_id) "
		"WHERE a.user_id=?"
	);

	_getAllBotScriptsStmt = makePreparedStatement(
		"SELECT u.id, u.username, sv.id, sv.code "
		"FROM core_activesnake a "
		"LEFT JOIN core_snakeversion sv ON (sv.id=a.version_id) "
		"LEFT JOIN auth_user u ON (u.id=a.user_id)"
	);

	_getActiveBotIdsStmt = makePreparedStatement(
		"SELECT user_id FROM core_activesnake"
	);

	_getActiveCommandsStmt = makePreparedStatement(
		"SELECT id, user_id, command FROM core_servercommand WHERE ISNULL(result) ORDER BY dt_created"
	);

	_commandCompletedStmt = makePreparedStatement(
		"UPDATE core_servercommand SET result=?, result_msg=?, dt_processed=UTC_TIMESTAMP() WHERE id=?"
	);

	_reportBotKilledStmt = makePreparedStatement(
		"INSERT INTO core_snakegame "
		" (user_id, snake_version_id, start_frame, end_frame, killer_id, final_mass, end_date) "
		"VALUES "
		" (?, ?, ?, ?, ?, ?, UTC_TIMESTAMP())"
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

std::vector<Command> MysqlDatabase::GetActiveCommands()
{
	std::unique_ptr<sql::ResultSet> res(_getActiveCommandsStmt->executeQuery());
	std::vector<Command> retval;
	while (res->next())
	{
		retval.emplace_back(res->getInt64(1), res->getInt64(2), res->getString(3));
	}
	return retval;
}

void MysqlDatabase::SetCommandCompleted(long commandId, bool result, std::string resultMsg)
{
	_commandCompletedStmt->setBoolean(1, result);
	_commandCompletedStmt->setString(2, resultMsg);
	_commandCompletedStmt->setInt64(3, commandId);
	_commandCompletedStmt->execute();
}

void MysqlDatabase::ReportBotKilled(long victim_id, long version_id, long start_frame, long end_frame, long killer_id, double final_mass)
{
	_reportBotKilledStmt->setInt64(1, victim_id);
	_reportBotKilledStmt->setInt64(2, version_id);
	_reportBotKilledStmt->setInt64(3, start_frame);
	_reportBotKilledStmt->setInt64(4, end_frame);
	_reportBotKilledStmt->setInt64(5, killer_id);
	if (killer_id<0) { _reportBotKilledStmt->setNull(5, 0); }
	_reportBotKilledStmt->setDouble(6, final_mass);
	_reportBotKilledStmt->execute();
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

std::unique_ptr<sql::PreparedStatement> MysqlDatabase::makePreparedStatement(std::string sql)
{
	return std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(sql)
	);
}

