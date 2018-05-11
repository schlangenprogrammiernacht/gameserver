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

#include <memory>

#include "IdentifyableObject.h"

#include "types.h"
#include "PositionObject.h"

class Field;
class Bot;

/*!
 * A piece of food that can be eaten by snakes.
 */
class Food : public IdentifyableObject, public PositionObject
{
	public:
		/*!
		 * Creates a new food pice at the given position and of the given value.
		 */
		Food(bool shallRegenerate, const Vector2D &pos, real_t value,
				const std::shared_ptr<Bot> &hunter = nullptr);

		bool decay(void);
		bool hasDecayed(void) const;
		real_t getValue() const { return m_value; }
		bool shallRegenerate() const { return m_shallRegenerate; }
		bool shallBeRemoved() const { return m_shallBeRemoved; }
		void markForRemove() { m_shallBeRemoved = true; }

		/*!
		 * Get the hunting Bot causing this Food to be created.
		 *
		 * This will return a nullptr for Food which spawned "naturally" or from boosting Bots.
		 *
		 * \returns A shared pointer to the hunting Bot.
		 */
		std::shared_ptr<Bot> getHunter(void) const { return m_hunter; }

	private:
		real_t  m_value;
		bool m_shallRegenerate;
		bool m_shallBeRemoved;

		std::shared_ptr<Bot> m_hunter;
};
