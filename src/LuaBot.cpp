#include "LuaBot.h"
#include "Food.h"
#include "Bot.h"
#include "Food.h"

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

bool LuaBot::step(Bot &bot, float &next_heading, bool &speed)
{
	float_t last_heading = bot.getHeading();
	float_t heading_rad = 2*M_PI * (last_heading / 360.0);
	next_heading = last_heading;
	speed = false;

	if (!m_initialized && !init())
	{
		return false;
	}


	auto food = m_lua_state.create_table();
	bot.getGlobalView().findFood(
		bot.getSnake()->getHeadPosition(),
		100*bot.getSnake()->getSegmentRadius(),
		[this, &food, heading_rad](const Vector& pos, const GlobalView::FoodInfo& foodinfo) {
			float_t direction = atan2(pos.y(), pos.x()) - heading_rad;
			while (direction<0) { direction += 2*M_PI; }
			food.add(
				m_lua_state.create_table_with(
					"x", pos.x(),
					"y", pos.y(),
					"v", foodinfo.food->getValue(),
					"d", direction,
					"dist", pos.abs()
				)
			);
		}
	);

	try
	{
		setQuota(1000000, 0.1);
		next_heading = m_lua_safe_env["step"](food);
		next_heading *= (360.0/2*M_PI);
		next_heading += last_heading;
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
	env["log"] = [](std::string v) { printf("%s\n", v.c_str()); };
	env["ipairs"] = m_lua_state["ipairs"];
	env["math"] = createFunctionTable(
		"math", std::vector<std::string> {
			"abs", "acos", "asin", "atan", "atan2",
			"ceil", "cos", "cosh", "deg", "exp",
			"floor", "fmod", "frexp", "huge",
			"ldexp", "log", "log10", "max", "min", "modf",
			"pi", "pow", "rad", "random",
			"sin", "sinh", "sqrt", "tan", "tanh"
		}
	);
	env["os"] = createFunctionTable(
		"os", std::vector<std::string> {
			"clock", "difftime", "time"
		}
	);
	env["string"] = createFunctionTable(
		"string", std::vector<std::string> {
			"format", "sub", "upper", "lower"
		}
	);
	return env;
}

sol::table LuaBot::createFunctionTable(const std::string &obj, const std::vector<std::string> &items)
{
	auto table = m_lua_state.create_table();
	for (auto item: items)
	{
		table.set(item, m_lua_state[obj][item]);
	}
	return table;
}
