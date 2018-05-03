#pragma once

#include <sol_forward.hpp>
#include <types.h>

namespace sol { class state; }

struct LuaSegmentInfo
{
	real_t x, y, r, d, dist;
	guid_t bot;

	LuaSegmentInfo(real_t aX,real_t aY, real_t aR, real_t aD, real_t aDist, guid_t aBot);
	static void Register(sol::state& lua);
};
