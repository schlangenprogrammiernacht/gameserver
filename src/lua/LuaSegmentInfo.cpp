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
