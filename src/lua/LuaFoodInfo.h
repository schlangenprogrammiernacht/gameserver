#pragma once

#include <sol.hpp>
#include <types.h>

struct LuaFoodInfo
{
	real_t x, y, v, d, dist;
	LuaFoodInfo(real_t aX, real_t aY, real_t aV, real_t aD, real_t aDist);
	static void Register(sol::state& lua);
};
