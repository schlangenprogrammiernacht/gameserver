#include "LuaBot.h"
#include "Food.h"
#include "Bot.h"
#include "Food.h"
#include "Field.h"

LuaBot::LuaBot(Bot &bot)
	: m_bot(bot)
{
	LuaFoodInfo::Register(m_lua_state);
	m_luaFoodInfoTable.reserve(1000);

	LuaSegmentInfo::Register(m_lua_state);
	m_luaSegmentInfoTable.reserve(1000);
}

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

bool LuaBot::step(float &next_heading, bool &boost)
{
	float_t last_heading = m_bot.getHeading();
	next_heading = last_heading;
	boost = false;

	if (!m_initialized && !init())
	{
		return false;
	}

	m_lua_safe_env["self"] = m_lua_state.create_table_with(
		"id", m_bot.getGUID(),
		"r",  m_bot.getSnake()->getSegmentRadius()
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
	env["findFood"] = [this](float_t radius, float_t min_size) { return apiFindFood(radius, min_size); };
	env["findSegments"] = [this](float_t radius, bool include_self) { return apiFindSegments(radius, include_self); };

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

std::vector<LuaFoodInfo>& LuaBot::apiFindFood(float_t radius, float_t min_size)
{
	auto head_pos = m_bot.getSnake()->getHeadPosition();
	float_t heading_rad = static_cast<float_t>(2.0 * M_PI * (m_bot.getHeading() / 360.0));

	radius = std::min(radius, getMaxSightRadius());

	m_luaFoodInfoTable.clear();
	auto field = m_bot.getField();
	field->getFoodInfoMap().processElements(
		head_pos,
		radius,
		[this, field, head_pos, heading_rad, min_size](const Field::FoodInfo& foodinfo)
		{
			if (foodinfo.food->getValue()>=min_size)
			{
				Vector2D relPos = field->unwrapRelativeCoords(foodinfo.pos() - head_pos);
				float_t direction = static_cast<float_t>(atan2(relPos.y(), relPos.x())) - heading_rad;
				while (direction<0) { direction += 2*M_PI; }
				while (direction>2*M_PI) { direction -= 2*M_PI; }
				m_luaFoodInfoTable.emplace_back(
					relPos.x(),
					relPos.y(),
					foodinfo.food->getValue(),
					direction,
					relPos.norm()
				);
			}
			return true;
		}
	);

	return m_luaFoodInfoTable;
}

std::vector<LuaSegmentInfo>& LuaBot::apiFindSegments(float_t radius, bool include_self)
{
	auto pos = m_bot.getSnake()->getHeadPosition();
	float_t heading_rad = 2*M_PI * (m_bot.getHeading() / 360.0);
	radius = std::min(radius, getMaxSightRadius());
	auto self_id = m_bot.getGUID();

	m_luaSegmentInfoTable.clear();
	auto field = m_bot.getField();
	field->getSegmentInfoMap().processElements(
		pos,
		radius,
		[this, field, pos, heading_rad, self_id, include_self](const Field::SnakeSegmentInfo& segmentInfo)
		{
			if (!include_self && (segmentInfo.bot->getGUID() == self_id)) { return true; }

			Vector2D relPos = field->unwrapRelativeCoords(segmentInfo.pos() - pos);
			float_t direction = atan2(relPos.y(), relPos.x()) - heading_rad;
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
			return true;
		}
	);

	return m_luaSegmentInfoTable;
}

float_t LuaBot::getMaxSightRadius() const
{
	return 50.0f + 15.0f * m_bot.getSnake()->getSegmentRadius();
}
