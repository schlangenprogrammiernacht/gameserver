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

#include "LuaFoodInfo.h"
#include <sol.hpp>

LuaFoodInfo::LuaFoodInfo(real_t aX, real_t aY, real_t aV, real_t aD, real_t aDist)
	: x(aX), y(aY), v(aV), d(aD), dist(aDist)
{
}

void LuaFoodInfo::Register(sol::state &lua)
{
	lua.new_usertype<LuaFoodInfo>(
		"FoodInfo",
		"x", sol::readonly(&LuaFoodInfo::x),
		"y", sol::readonly(&LuaFoodInfo::y),
		"v", sol::readonly(&LuaFoodInfo::v),
		"d", sol::readonly(&LuaFoodInfo::d),
		"dist", sol::readonly(&LuaFoodInfo::dist)
	);
}
