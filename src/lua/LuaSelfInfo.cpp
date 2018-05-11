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

#include "LuaSelfInfo.h"
#include <map>
#include <sol.hpp>
#include <config.h>
#include <Bot.h>
#include <Field.h>

LuaSelfInfo::LuaSelfInfo(Bot &bot, guid_t aId)
	: id(aId), colorsLocked(false), m_bot(bot)
{
	m_cachedColors.reserve(config::MAX_COLORS);
}

void LuaSelfInfo::setColorTable(sol::table v)
{
	if(v == sol::nil) {
		return;
	}

	m_colorTable = v;
	m_cachedColors.clear();
	for (auto kvp: v)
	{
		auto index = kvp.first.as<size_t>();
		auto value = kvp.second.as<uint32_t>();
		if ((index>=1) && (index<=config::MAX_COLORS))
		{
			while (m_cachedColors.size() < index)
			{
				m_cachedColors.push_back(0x808080);
			}
			m_cachedColors[index-1] = value;
		}
	}
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
		"max_step_angle", sol::property(&LuaSelfInfo::getMaxStepAngle),
		"start_frame", sol::property(&LuaSelfInfo::getStartFrame),
		"current_frame", sol::property(&LuaSelfInfo::getCurrentFrame),
		"speed", sol::property(&LuaSelfInfo::getSpeed),
		"food_consumed_natural", sol::property(&LuaSelfInfo::getConsumedNaturalFood),
		"food_consumed_hunted_self", sol::property(&LuaSelfInfo::getConsumedFoodHuntedBySelf),
		"food_consumed_hunted_by_others", sol::property(&LuaSelfInfo::getConsumedFoodHuntedByOthers),
		"colors", sol::property(&LuaSelfInfo::getColorTable, &LuaSelfInfo::setColorTable),
		"face", sol::property(&LuaSelfInfo::getFace, &LuaSelfInfo::setFace),
		"logo", sol::property(&LuaSelfInfo::getDogTag, &LuaSelfInfo::setDogTag)
	);
}

void LuaSelfInfo::setFace()
{
	if (colorsLocked)
	{
		throw std::runtime_error("faces can only be changed in the init() function");
	}
}

void LuaSelfInfo::setDogTag()
{
	if (colorsLocked)
	{
		throw std::runtime_error("dog tags can only be changed in the init() function");
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

real_t LuaSelfInfo::getMaxStepAngle()
{
	return m_bot.getSnake()->maxRotationPerStep();
}

void LuaSelfInfo::registerColorTable(sol::table colorTable)
{
	m_colorTable = colorTable;
}
