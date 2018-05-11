/*
 * Schlangenprogrammiernacht: A programming game for GPN18.
 * Copyright (C) 2018  bytewerk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
