#pragma once

#include <sol_forward.hpp>
#include <types.h>

namespace sol { class state; }

class Bot;
struct LuaSegmentInfo
{
	public:
		real_t x, y, r, d, dist;
		LuaSegmentInfo(Bot* aBot, real_t aX,real_t aY, real_t aR, real_t aD, real_t aDist);
		static void Register(sol::state& lua);

		guid_t getBotId();
		std::string getBotName();

	private:
		Bot *m_bot;
};
