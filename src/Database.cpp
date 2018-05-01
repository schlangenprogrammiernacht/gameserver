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

	_getBotDataStmt = makePreparedStatement(
		"SELECT u.id, u.username, sv.id, sv.code, IFNULL(p.viewer_key, 0) AS viewer_key "
		"FROM core_userprofile p "
		"LEFT JOIN auth_user u ON (u.id=p.user_id) "
		"LEFT JOIN core_snakeversion sv ON (sv.id=p.active_snake_id) "
		"WHERE p.user_id=? AND p.active_snake_id IS NOT NULL"
	);

	_getActiveBotIdsStmt = makePreparedStatement(
		"SELECT user_id FROM core_userprofile WHERE active_snake_id IS NOT NULL"
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

	_disableBotVersionStmt = makePreparedStatement(
		"UPDATE core_userprofile SET active_snake_id=NULL WHERE active_snake_id=?"
	);

	_saveBotVersionErrorMessageStmt = makePreparedStatement(
		"UPDATE core_snakeversion SET server_error_message=? WHERE id=?"
	);

	_setupLiveStatsStmt = makePreparedStatement(
		"REPLACE INTO core_livestats "
		" (user_id) "
		"VALUES"
		" (?)"
	);

	_updateLiveStatsStmt = makePreparedStatement(
		"UPDATE core_livestats "
		"SET last_update_frame=?, mass=?, natural_food_consumed=?, carrison_food_consumed=?, hunted_food_consumed=? "
		"WHERE user_id=?"
	);

	_removeLiveStatsStmt = makePreparedStatement(
		"DELETE FROM core_livestats "
		"WHERE user_id=?"
	);
}

std::unique_ptr<BotScript> MysqlDatabase::GetBotData(int bot_id)
{
	_getBotDataStmt->setInt(1, bot_id);

	std::unique_ptr<sql::ResultSet> res(_getBotDataStmt->executeQuery());
	std::vector<BotScript> retval;
	if (!res->next())
	{
		return nullptr;
	}

	return std::make_unique<BotScript>(
		res->getInt(IDX_BOTSCRIPT_BOT_ID),
		res->getString(IDX_BOTSCRIPT_BOT_NAME),
		res->getInt(IDX_BOTSCRIPT_VERSION_ID),
		res->getInt64(IDX_BOTSCRIPT_VIEWER_KEY),
		res->getString(IDX_BOTSCRIPT_CODE)
	);
}

std::vector<int> MysqlDatabase::GetActiveBotIds()
{
	std::unique_ptr<sql::ResultSet> res(_getActiveBotIdsStmt->executeQuery());
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

void MysqlDatabase::DisableBotVersion(long version_id, std::string errorMessage)
{
	_disableBotVersionStmt->setInt64(1, version_id);
	_disableBotVersionStmt->execute();

	if (!errorMessage.empty())
	{
		_saveBotVersionErrorMessageStmt->setString(1, errorMessage);
		_saveBotVersionErrorMessageStmt->setInt64(2, version_id);
		_saveBotVersionErrorMessageStmt->execute();
	}
}

void MysqlDatabase::SetupLiveStats(long user_id)
{
	_setupLiveStatsStmt->setInt64(1, user_id);
	_setupLiveStatsStmt->execute();
}

void MysqlDatabase::UpdateLiveStats(long user_id, long current_frame, double mass,
		double natural_food_consumed, double carrison_food_consumed,
		double hunted_food_consumed)
{
	_updateLiveStatsStmt->setInt64(1, current_frame);
	_updateLiveStatsStmt->setDouble(2, mass);
	_updateLiveStatsStmt->setDouble(3, natural_food_consumed);
	_updateLiveStatsStmt->setDouble(4, carrison_food_consumed);
	_updateLiveStatsStmt->setDouble(5, hunted_food_consumed);
	_updateLiveStatsStmt->setInt64(6, user_id);
	_updateLiveStatsStmt->execute();
}

void MysqlDatabase::RemoveLiveStats(long user_id)
{
	_removeLiveStatsStmt->setInt64(1, user_id);
	_removeLiveStatsStmt->execute();
}

std::unique_ptr<sql::PreparedStatement> MysqlDatabase::makePreparedStatement(std::string sql)
{
	return std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(sql)
	);
}

