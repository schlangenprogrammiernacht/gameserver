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
