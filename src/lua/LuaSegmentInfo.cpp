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

#include "LuaSegmentInfo.h"
#include <sol.hpp>
#include <Bot.h>

LuaSegmentInfo::LuaSegmentInfo(Bot* aBot, real_t aX, real_t aY, real_t aR, real_t aD, real_t aDist)
	: x(aX), y(aY), r(aR), d(aD), dist(aDist), m_bot(aBot)
{
}

void LuaSegmentInfo::Register(sol::state &lua)
{
	lua.new_usertype<LuaSegmentInfo>(
		"SegmentInfo",
		"x", sol::readonly(&LuaSegmentInfo::x),
		"y", sol::readonly(&LuaSegmentInfo::y),
		"r", sol::readonly(&LuaSegmentInfo::r),
		"d", sol::readonly(&LuaSegmentInfo::d),
		"dist", sol::readonly(&LuaSegmentInfo::dist),
		"bot", sol::property(&LuaSegmentInfo::getBotId),
		"bot_id", sol::property(&LuaSegmentInfo::getBotId),
		"bot_name", sol::property(&LuaSegmentInfo::getBotName)
	);
}

guid_t LuaSegmentInfo::getBotId()
{
	return m_bot->getGUID();
}

std::string LuaSegmentInfo::getBotName()
{
	return m_bot->getName();
}
