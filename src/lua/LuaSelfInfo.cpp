#include "LuaSelfInfo.h"
#include <map>
#include <config.h>
#include <Bot.h>
#include <Field.h>

LuaSelfInfo::LuaSelfInfo(Bot &bot, guid_t aId)
	: id(aId), colorsLocked(false), m_bot(bot)
{
}

void LuaSelfInfo::Register(sol::state &lua)
{
	lua.new_usertype<LuaSelfInfo>(
		"SelfInfo",
		"id", sol::readonly(&LuaSelfInfo::id),
		"r", sol::property(&LuaSelfInfo::getRadius), // TODO remove?
		"segment_radius", sol::property(&LuaSelfInfo::getRadius),
		"mass", sol::property(&LuaSelfInfo::getMass),
		"sight_radius", sol::property(&LuaSelfInfo::getSightRadius),
		"consume_radius", sol::property(&LuaSelfInfo::getConsumeRadius),
		"start_frame", sol::property(&LuaSelfInfo::getStartFrame),
		"current_frame", sol::property(&LuaSelfInfo::getCurrentFrame),
		"speed", sol::property(&LuaSelfInfo::getSpeed),
		"food_consumed_natural", sol::property(&LuaSelfInfo::getConsumedNaturalFood),
		"food_consumed_hunted_self", sol::property(&LuaSelfInfo::getConsumedFoodHuntedBySelf),
		"food_consumed_hunted_by_others", sol::property(&LuaSelfInfo::getConsumedFoodHuntedByOthers),
		"colors", sol::property(&LuaSelfInfo::getColors, &LuaSelfInfo::setColors)
	);
}

void LuaSelfInfo::setColors(sol::table v)
{
	if (colorsLocked)
	{
		throw std::runtime_error("snake colors can only be changed in the init() function");
	}
	std::map<size_t,uint32_t> colorMap;
	v.for_each([&colorMap](sol::object key, sol::object value)
	{
		auto index = key.as<size_t>();
		if (index>0) { index--; } // lua tables tend to start by index 1
		if (index>config::MAX_COLORS) { return; }
		colorMap[index] = value.as<uint32_t>();
	});

	colors.clear();
	for (auto kvp: colorMap)
	{
		while (colors.size() < kvp.first)
		{
			colors.push_back(colors.empty() ? 0x808080 : colors.back());
		}
		colors.push_back(kvp.second);
	}
	if (colors.empty())
	{
		colors.push_back(0x0000FF00);
	}
}

real_t LuaSelfInfo::getRadius()
{
	return m_bot.getSnake()->getSegmentRadius();
}

real_t LuaSelfInfo::getMass()
{
	return m_bot.getSnake()->getMass();
}

real_t LuaSelfInfo::getSightRadius()
{
	return m_bot.getSightRadius();
}

real_t LuaSelfInfo::getConsumeRadius()
{
	return m_bot.getSnake()->getConsumeRadius();
}

uint32_t LuaSelfInfo::getStartFrame()
{
	return m_bot.getStartFrame();
}

uint32_t LuaSelfInfo::getCurrentFrame()
{
	return m_bot.getField()->getCurrentFrame();
}

real_t LuaSelfInfo::getSpeed()
{
	return config::SNAKE_DISTANCE_PER_STEP;
}

real_t LuaSelfInfo::getConsumedNaturalFood()
{
	return m_bot.getConsumedNaturalFood();
}

real_t LuaSelfInfo::getConsumedFoodHuntedByOthers()
{
	return m_bot.getConsumedFoodHuntedByOthers();
}

real_t LuaSelfInfo::getConsumedFoodHuntedBySelf()
{
	return m_bot.getConsumedFoodHuntedBySelf();
}
