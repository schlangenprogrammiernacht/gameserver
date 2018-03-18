#include "LuaBot.h"

bool LuaBot::init()
{
	try
	{
		m_lua_state.open_libraries();
		m_lua_state.script_file("lua/quota.lua");
		m_lua_safe_env = createEnvironment();
		m_lua_state.safe_script_file("lua/demobot.lua", m_lua_safe_env);
		m_initialized = true;
		return true;
	}
	catch (const sol::error& e)
	{
		m_initialized = false;
		printf("init failed: %s\n", e.what());
		return false;
	}
}

bool LuaBot::step(float last_heading, float &next_heading, bool &speed)
{
	next_heading = last_heading;
	speed = false;

	if (!m_initialized && !init())
	{
		return false;
	}

	try
	{
		setQuota(1000000, 0.1);
		next_heading = m_lua_safe_env["run"](last_heading);
		return true;
	}
	catch (const sol::error& e)
	{
		printf("script aborted: %s\n", e.what());
		return false;
	}
}

void LuaBot::setQuota(uint32_t num_instructions, double seconds)
{
	m_lua_state["set_quota"](num_instructions, seconds);
}

sol::environment LuaBot::createEnvironment()
{
	auto env = sol::environment(m_lua_state, sol::create);
	env["math"] = m_lua_state.create_table_with(
		"random", m_lua_state["math"]["random"],
		"sin", m_lua_state["math"]["sin"]
	);
	return env;
}
