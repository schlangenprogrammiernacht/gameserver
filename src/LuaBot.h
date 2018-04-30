#pragma once
#include <sol.hpp>
#include "types.h"
#include "config.h"
#include "PoolAllocator.h"

struct LuaSegmentInfo
{
	real_t x, y, r, d, dist;
	guid_t bot;

	LuaSegmentInfo(real_t aX,real_t aY, real_t aR, real_t aD, real_t aDist, guid_t aBot)
		: x(aX), y(aY), r(aR), d(aD), dist(aDist), bot(aBot)
	{
	}

	static void Register(sol::state& lua)
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
};

struct LuaFoodInfo
{
	real_t x, y, v, d, dist;
	LuaFoodInfo(real_t aX, real_t aY, real_t aV, real_t aD, real_t aDist)
		: x(aX), y(aY), v(aV), d(aD), dist(aDist)
	{
	}

	static void Register(sol::state& lua)
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
};

struct LuaSelfInfo
{
	guid_t id;
	real_t r;
	std::vector<uint32_t> colors;

	LuaSelfInfo(guid_t aId, real_t radius)
		: id(aId), r(radius)
	{}

	std::vector<uint32_t>& getColors()
	{
		return colors;
	}

	void setColors(sol::table v)
	{
		std::map<size_t,uint32_t> colorMap;
		v.for_each([&colorMap](sol::object key, sol::object value)
		{
			auto index = key.as<size_t>();
			if (index>0) { index--; } // lua tables tend to start by index 1
			if (index>config::MAX_COLORS) { return; }
			colorMap[index] = value.as<uint32_t>();
		});

		colors.clear();
		for (auto kvp: colorMap)
		{
			while (colors.size() < kvp.first)
			{
				colors.push_back(colors.empty() ? 0x808080 : colors.back());
			}
			colors.push_back(kvp.second);
		}
		if (colors.empty())
		{
			colors.push_back(0x0000FF00);
		}
	}

	static void Register(sol::state& lua)
	{
		lua.new_usertype<LuaSelfInfo>(
			"SelfInfo",
			"id", sol::readonly(&LuaSelfInfo::id),
			"r", sol::readonly(&LuaSelfInfo::r),
			"colors", sol::property(&LuaSelfInfo::getColors, &LuaSelfInfo::setColors)
		);
	}
};

class Bot;
class LuaBot
{
	public:
		LuaBot(Bot &bot, std::string script);
		bool init(std::string &initErrorMessage);
		bool step(float &next_heading, bool &boost);
		std::vector<uint32_t> getColors() { return m_self.colors; }

	private:
		Bot& m_bot;
		PoolAllocator m_allocator;
		sol::state m_lua_state;
		sol::environment m_lua_safe_env;
		std::vector<LuaFoodInfo> m_luaFoodInfoTable;
		std::vector<LuaSegmentInfo> m_luaSegmentInfoTable;
		LuaSelfInfo m_self;
		std::string m_script;

		void setQuota(uint32_t num_instructions, double seconds);
		sol::environment createEnvironment();
		sol::table createFunctionTable(const std::string& obj, const std::vector<std::string>& items);

		std::vector<LuaFoodInfo>& apiFindFood(real_t radius, real_t min_size);
		std::vector<LuaSegmentInfo>& apiFindSegments(real_t radius, bool include_self);
		bool apiLog(std::string data);
		void apiCallInit();

		real_t getMaxSightRadius() const;

};
