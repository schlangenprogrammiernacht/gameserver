#pragma once

#include <sol_forward.hpp>
#include <types.h>

namespace sol { class state; }

struct LuaFoodInfo
{
	real_t x, y, v, d, dist;
	LuaFoodInfo(real_t aX, real_t aY, real_t aV, real_t aD, real_t aDist);
	static void Register(sol::state& lua);
};
