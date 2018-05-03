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
