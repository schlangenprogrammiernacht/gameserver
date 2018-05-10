#pragma once
#include <types.h>
#include <vector>
#include <sol.hpp>

class Bot;

namespace sol { class state; }

class LuaSelfInfo
{
	public:
		guid_t id;
		bool colorsLocked;
		uint32_t face = 0;
		uint32_t dogTag = 0;

		LuaSelfInfo(Bot& bot, guid_t aId);

		sol::table getColorTable() { return m_colorTable; }
		void setColorTable(sol::table v);

		uint32_t getFace() { return face; }
		void setFace();

		uint32_t getDogTag() { return dogTag; }
		void setDogTag();

		real_t getRadius();
		real_t getMass();
		real_t getSightRadius();
		real_t getConsumeRadius();
		uint32_t getStartFrame();
		uint32_t getCurrentFrame();
		real_t getSpeed();
		real_t getConsumedNaturalFood();
		real_t getConsumedFoodHuntedByOthers();
		real_t getConsumedFoodHuntedBySelf();
		real_t getMaxStepAngle();

		void registerColorTable(sol::table colorTable);

		static void Register(sol::state& lua);

		std::vector<uint32_t>& getCachedColors() { return m_cachedColors; }

	private:
		Bot &m_bot;
		sol::table m_colorTable;
		std::vector<uint32_t> m_cachedColors;
};

