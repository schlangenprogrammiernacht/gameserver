#pragma once
#include <sol.hpp>

class Bot;

class LuaBot
{
	public:
		LuaBot(Bot& bot);
		bool init();
		bool step(float &next_heading, bool &boost);

	private:
		Bot& m_bot;
		bool m_initialized = false;
		sol::state m_lua_state;
		sol::environment m_lua_safe_env;

		void setQuota(uint32_t num_instructions, double seconds);
		sol::environment createEnvironment();
		sol::table createFunctionTable(const std::string& obj, const std::vector<std::string>& items);

		std::vector<sol::table> apiFindFood(float_t radius, float_t min_size);
		std::vector<sol::table> apiFindSegments(float_t radius, bool include_self);

		float_t getMaxSightRadius() const;

};
