#include "LuaSegmentInfo.h"
#include <sol.hpp>

LuaSegmentInfo::LuaSegmentInfo(real_t aX, real_t aY, real_t aR, real_t aD, real_t aDist, guid_t aBot)
	: x(aX), y(aY), r(aR), d(aD), dist(aDist), bot(aBot)
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
		"bot", sol::readonly(&LuaSegmentInfo::bot)
	);
}
