#include "LuaBot.h"
#include "Food.h"
#include "Bot.h"
#include "Food.h"
#include "Field.h"
#include "config.h"
#include <iostream>

LuaBot::LuaBot(Bot &bot, std::string script)
	: m_bot(bot)
	, m_allocator(config::LUA_MEM_POOL_SIZE_BYTES, config::LUA_MEM_POOL_BLOCK_SIZE_BYTES)
	, m_lua_state(sol::default_at_panic, PoolAllocator::lua_allocator, &m_allocator)
	, m_self(bot, bot.getGUID())
	, m_script(script)
{
	LuaFoodInfo::Register(m_lua_state);
	m_luaFoodInfoTable.reserve(1000);

	LuaSegmentInfo::Register(m_lua_state);
	m_luaSegmentInfoTable.reserve(1000);

	LuaSelfInfo::Register(m_lua_state);
	m_self.colors.push_back(0x0000FF);
}

bool LuaBot::init(std::string& initErrorMessage)
{
	try
	{
		m_lua_state.open_libraries();
		m_lua_state.script_file("lua/quota.lua");

		m_setQuotaFunc = m_lua_state["set_quota"];
		m_clearQuotaFunc = m_lua_state["clear_quota"];
		m_lua_safe_env = createEnvironment();

		std::string chunkName = "bot.lua";
		m_lua_state.safe_script(m_script, m_lua_safe_env, chunkName, sol::load_mode::text);

		auto step_function = m_lua_safe_env["step"];
		if (step_function.get_type() != sol::type::function)
		{
			throw std::runtime_error("script does not define a step() function.");
		}

		apiCallInit();
		m_self.colorsLocked = true;

		return true;
	}
	catch (const std::runtime_error& e)
	{
		initErrorMessage = e.what();
		printf("init failed: %s\n", e.what());
		return false;
	}
}

bool LuaBot::step(float &next_heading, bool &boost)
{
	real_t last_heading = m_bot.getHeading();
	next_heading = last_heading;
	boost = false;

	bool retval = false;

	try {
		setQuota(1000000, 0.1);
		sol::protected_function step = m_lua_safe_env["step"];

		auto result = step();
		if (result.valid())
		{
			next_heading = result.get<real_t>(0);

			if ((result.return_count()<1) || (result.return_count()>2))
			{
				throw std::runtime_error("step() must return a direction (as float, in radiens) and optionally if the bot should boost (as bool)");
			}

			if (!std::isfinite(next_heading))
			{
				throw std::runtime_error("step() did not return a finite number. did you divide by zero or return a string?");
			}

			boost = (result.return_count()>1) && result.get<bool>(1);

			next_heading = result.get<float>(0);
			next_heading = 180 * (next_heading / M_PI);
			next_heading += last_heading;
			retval = true;
		}
		else
		{
			sol::error err = result;
			m_bot.appendLogMessage(err.what(), false);
		}

		clearQuota();
	}
	catch (std::runtime_error err)
	{
		clearQuota();
		m_bot.appendLogMessage(err.what(), false);
		retval = false;
	}

	return retval;
}

void LuaBot::setQuota(uint32_t num_instructions, double seconds)
{
	m_setQuotaFunc(num_instructions, seconds);
}

void LuaBot::clearQuota()
{
	m_clearQuotaFunc();
}

sol::environment LuaBot::createEnvironment()
{
	auto env = sol::environment(m_lua_state, sol::create);
	env["self"] = &m_self;
	env["findFood"] = [this](real_t radius, real_t min_size) { return apiFindFood(radius, min_size); };
	env["findSegments"] = [this](real_t radius, bool include_self) { return apiFindSegments(radius, include_self); };
	env["log"] = [this](std::string v) { return apiLog(v); };

	for (auto& func: std::vector<std::string>{
		"assert", "print", "ipairs", "error", "next", "pairs", "pcall", "select",
		"tonumber", "tostring", "type", "unpack", "_VERSION", "xpcall"
	})
	{
		env[func] = m_lua_state[func];
	}
	env["math"] = createFunctionTable(
		"math", std::vector<std::string> {
			"abs", "acos", "asin", "atan", "atan2",
			"ceil", "cos", "cosh", "deg", "exp",
			"floor", "fmod", "frexp", "huge",
			"ldexp", "log", "log10", "max", "min", "modf",
			"pi", "pow", "rad", "random", "randomseed",
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
	env["table"] = createFunctionTable(
		"table", std::vector<std::string> {
			"getn", "setn", "maxn", "insert", "remove", "sort"
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

std::vector<LuaFoodInfo>& LuaBot::apiFindFood(real_t radius, real_t min_size)
{
	m_luaFoodInfoTable.clear();

	auto head_pos = m_bot.getSnake()->getHeadPosition();
	real_t heading_rad = static_cast<real_t>(2.0 * M_PI * (m_bot.getHeading() / 360.0));

	radius = std::min(radius, m_bot.getSightRadius());

	auto field = m_bot.getField();
	for (auto &food: field->getFoodMap().getRegion(head_pos, radius))
	{
		if (food.getValue()>=min_size)
		{
			Vector2D relPos = field->unwrapRelativeCoords(food.pos() - head_pos);
			real_t direction = static_cast<real_t>(atan2(relPos.y(), relPos.x())) - heading_rad;
			m_luaFoodInfoTable.emplace_back(
				relPos.x(),
				relPos.y(),
				food.getValue(),
				direction,
				relPos.norm()
			);
		}
	}

	return m_luaFoodInfoTable;
}

std::vector<LuaSegmentInfo>& LuaBot::apiFindSegments(real_t radius, bool include_self)
{
	m_luaSegmentInfoTable.clear();

	auto pos = m_bot.getSnake()->getHeadPosition();
	real_t heading_rad = 2*M_PI * (m_bot.getHeading() / 360.0);
	radius = std::min(radius, m_bot.getSightRadius());
	auto self_id = m_bot.getGUID();

	auto field = m_bot.getField();
	for (auto &segmentInfo: field->getSegmentInfoMap().getRegion(pos, radius + m_bot.getField()->getMaxSegmentRadius()))
	{
		if (!include_self && (segmentInfo.bot->getGUID() == self_id)) { continue; }		
		real_t segmentRadius = segmentInfo.bot->getSnake()->getSegmentRadius();
		Vector2D relPos = field->unwrapRelativeCoords(segmentInfo.pos() - pos);
		real_t distance = relPos.norm();
		if (distance > (radius+segmentRadius)) { continue; }

		real_t direction = atan2(relPos.y(), relPos.x()) - heading_rad;
		m_luaSegmentInfoTable.emplace_back(
			relPos.x(),
			relPos.y(),
			segmentInfo.bot->getSnake()->getSegmentRadius(),
			direction,
			distance,
			segmentInfo.bot->getGUID()
		);
	}

	return m_luaSegmentInfoTable;
}

bool LuaBot::apiLog(std::string data)
{
	return m_bot.appendLogMessage(data, true);
}

void LuaBot::apiCallInit()
{
	sol::protected_function init_func = m_lua_safe_env["init"];
	if (init_func.get_type() != sol::type::function)
	{
		/* undefined init() is okay */
		return;
	}

	try {
		setQuota(1000000, 0.1);
		auto result = init_func();
		if (!result.valid())
		{
			sol::error err = result;
			throw std::runtime_error(err.what());
		}
		clearQuota();
	} catch (std::runtime_error err)
	{
		clearQuota();
		throw;
	}
}
