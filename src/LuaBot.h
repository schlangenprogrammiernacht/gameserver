#pragma once
#include <sol.hpp>

class LuaBot
{
	public:
		bool init();
		bool step(float last_heading, float &next_heading, bool &speed);

	private:
		bool m_initialized = false;
		sol::state m_lua_state;
		sol::environment m_lua_safe_env;

		void setQuota(uint32_t num_instructions, double seconds);
		sol::environment createEnvironment();
		sol::table createFunctionTable(const std::string& obj, const std::vector<std::string>& items);

};
