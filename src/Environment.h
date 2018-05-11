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
#include <cstdlib>

class Environment
{
	public:
		static constexpr const char* ENV_MYSQL_HOST = "MYSQL_HOST";
		static constexpr const char* ENV_MYSQL_HOST_DEFAULT = "localhost";

		static constexpr const char* ENV_MYSQL_USER = "MYSQL_USER";
		static constexpr const char* ENV_MYSQL_USER_DEFAULT = "gameserver";

		static constexpr const char* ENV_MYSQL_PASSWORD = "MYSQL_PASSWORD";
		static constexpr const char* ENV_MYSQL_PASSWORD_DEFAULT = "gameserver";

		static constexpr const char* ENV_MYSQL_DB = "MYSQL_DB";
		static constexpr const char* ENV_MYSQL_DB_DEFAULT = "gameserver";

		static const char* GetDefault(const char* env, const char* defaultValue)
		{
			const char* value = std::getenv(env);
			return (value!=nullptr) ? value : defaultValue;
		}
};
