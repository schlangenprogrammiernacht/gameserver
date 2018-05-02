#pragma once

#include <sol.hpp>
#include <types.h>

struct LuaSegmentInfo
{
	real_t x, y, r, d, dist;
	guid_t bot;

	LuaSegmentInfo(real_t aX,real_t aY, real_t aR, real_t aD, real_t aDist, guid_t aBot);
	static void Register(sol::state& lua);
};

