#pragma once
#include <sol.hpp>
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
		bool step(float &directionChange, bool &boost);
		std::vector<uint32_t> &getColors();
		uint32_t getFace() { return m_self.getFace(); }
		uint32_t getDogTag() { return m_self.getDogTag(); }

	private:
		Bot& m_bot;
		PoolAllocator m_allocator;
		sol::state m_lua_state;
		sol::environment m_lua_safe_env;
		std::vector<LuaFoodInfo> m_luaFoodInfoTable;
		std::vector<LuaSegmentInfo> m_luaSegmentInfoTable;
		sol::protected_function m_setQuotaFunc;
		sol::protected_function m_clearQuotaFunc;
		LuaSelfInfo m_self;
		std::string m_script;

		void setQuota(uint32_t num_instructions, double seconds);
		void clearQuota();
		sol::environment createEnvironment();
		sol::table createFunctionTable(const std::string& obj, const std::vector<std::string>& items);

		std::vector<LuaFoodInfo>& apiFindFood(real_t radius, real_t min_size);
		std::vector<LuaSegmentInfo>& apiFindSegments(real_t radius, bool include_self);
		bool apiLog(std::string data);
		void apiCallInit();

};
