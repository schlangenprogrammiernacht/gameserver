#pragma once
#include "config.h"
#include "PoolAllocator.h"
#include "lua/LuaSelfInfo.h"
#include "lua/LuaFoodInfo.h"
#include "lua/LuaSegmentInfo.h"

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

};
