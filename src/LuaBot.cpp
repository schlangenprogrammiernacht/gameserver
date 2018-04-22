#include "LuaBot.h"
#include "Food.h"
#include "Bot.h"
#include "Food.h"
#include "Field.h"

LuaBot::LuaBot()
{
	LuaFoodInfo::Register(m_lua_state);
	m_luaFoodInfoTable.reserve(1000);

	LuaSegmentInfo::Register(m_lua_state);
	m_luaSegmentInfoTable.reserve(1000);
}

bool LuaBot::init(std::string script)
{
	try
	{
		m_lua_state.open_libraries();
		m_lua_state.script_file("lua/quota.lua");
		m_lua_safe_env = createEnvironment();
		m_lua_state.safe_script(script, m_lua_safe_env);
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

bool LuaBot::step(Bot &bot, float &next_heading, bool &boost)
{
	if (!m_initialized)
	{
		return false;
	}

	m_bot = &bot;

	real_t last_heading = bot.getHeading();
	next_heading = last_heading;
	boost = false;

	m_lua_safe_env["self"] = m_lua_state.create_table_with(
		"id", bot.getGUID(),
		"r",  bot.getSnake()->getSegmentRadius()
	);

	try
	{
		setQuota(1000000, 0.1);
		next_heading = m_lua_safe_env["step"]();
		next_heading = 180 * (next_heading / M_PI);
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
	env["findFood"] = [this](real_t radius, real_t min_size) { return apiFindFood(radius, min_size); };
	env["findSegments"] = [this](real_t radius, bool include_self) { return apiFindSegments(radius, include_self); };

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

std::vector<LuaFoodInfo>& LuaBot::apiFindFood(real_t radius, real_t min_size)
{
	m_luaFoodInfoTable.clear();
	if (m_bot == nullptr) { return m_luaFoodInfoTable; }

	auto head_pos = m_bot->getSnake()->getHeadPosition();
	real_t heading_rad = static_cast<real_t>(2.0 * M_PI * (m_bot->getHeading() / 360.0));

	radius = std::min(radius, getMaxSightRadius());

	auto field = m_bot->getField();
	for (auto &food: field->getFoodMap().getRegion(head_pos, radius))
	{
		if (food.getValue()>=min_size)
		{
			Vector2D relPos = field->unwrapRelativeCoords(food.pos() - head_pos);
			real_t direction = static_cast<real_t>(atan2(relPos.y(), relPos.x())) - heading_rad;
			while (direction<0) { direction += 2*M_PI; }
			while (direction>2*M_PI) { direction -= 2*M_PI; }
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
	if (m_bot==nullptr) { return m_luaSegmentInfoTable; }

	auto pos = m_bot->getSnake()->getHeadPosition();
	real_t heading_rad = 2*M_PI * (m_bot->getHeading() / 360.0);
	radius = std::min(radius, getMaxSightRadius());
	auto self_id = m_bot->getGUID();

	auto field = m_bot->getField();
	for (auto &segmentInfo: field->getSegmentInfoMap().getRegion(pos, radius))
	{
		if (!include_self && (segmentInfo.bot->getGUID() == self_id)) { continue; }
		Vector2D relPos = field->unwrapRelativeCoords(segmentInfo.pos() - pos);
		real_t direction = atan2(relPos.y(), relPos.x()) - heading_rad;
		while (direction<0) { direction += 2*M_PI; }
		while (direction>2*M_PI) { direction -= 2*M_PI; }
		m_luaSegmentInfoTable.emplace_back(
			relPos.x(),
			relPos.y(),
			segmentInfo.bot->getSnake()->getSegmentRadius(),
			direction,
			relPos.norm(),
			segmentInfo.bot->getGUID()
		);
	}

	return m_luaSegmentInfoTable;
}

real_t LuaBot::getMaxSightRadius() const
{
	if (m_bot==nullptr) { return 0; }
	return 50.0f + 15.0f * m_bot->getSnake()->getSegmentRadius();
}
