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
		"SELECT u.id, u.username, sv.id, sv.code, sv.compile_state, IFNULL(p.viewer_key, 0) AS viewer_key "
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
		" (user_id, snake_version_id, start_frame, end_frame, killer_id, final_mass, natural_food_consumed, carrison_food_consumed, hunted_food_consumed, end_date) "
		"VALUES "
		" (?, ?, ?, ?, ?, ?, ?, ?, ?, UTC_TIMESTAMP())"
	);

	_disableBotVersionStmt = makePreparedStatement(
		"UPDATE core_userprofile SET active_snake_id=NULL WHERE active_snake_id=?"
	);

	_saveBotVersionErrorMessageStmt = makePreparedStatement(
		"UPDATE core_snakeversion SET server_error_message=? WHERE id=?"
	);

	_setBotToCrashedStateStmt = makePreparedStatement(
		"UPDATE core_snakeversion SET compile_state='crashed' WHERE id=?"
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
		res->getString(IDX_BOTSCRIPT_CODE),
		res->getString(IDX_BOTSCRIPT_COMPILE_STATE)
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

void MysqlDatabase::ReportBotKilled(long victim_id, long version_id, long start_frame, long end_frame, long killer_id, double final_mass, double natural_food_consumed, double carrison_food_consumed, double hunted_food_consumed)
{
	_reportBotKilledStmt->setInt64(1, victim_id);
	_reportBotKilledStmt->setInt64(2, version_id);
	_reportBotKilledStmt->setInt64(3, start_frame);
	_reportBotKilledStmt->setInt64(4, end_frame);
	_reportBotKilledStmt->setInt64(5, killer_id);
	if (killer_id<0) { _reportBotKilledStmt->setNull(5, 0); }
	_reportBotKilledStmt->setDouble(6, final_mass);
	_reportBotKilledStmt->setDouble(7, natural_food_consumed);
	_reportBotKilledStmt->setDouble(8, carrison_food_consumed);
	_reportBotKilledStmt->setDouble(9, hunted_food_consumed);
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

void MysqlDatabase::SetBotToCrashedState(long version_id)
{
	_setBotToCrashedStateStmt->setInt64(1, version_id);
	_setBotToCrashedStateStmt->execute();
}

std::unique_ptr<sql::PreparedStatement> MysqlDatabase::makePreparedStatement(std::string sql)
{
	return std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(sql)
	);
}
