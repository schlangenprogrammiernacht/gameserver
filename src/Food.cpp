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

#include "config.h"

#include "Food.h"

Food::Food(bool shallRegenerate, const Vector2D &pos, real_t value,
		const std::shared_ptr<Bot> &hunter)
	: PositionObject(pos)
	, m_value(value)
	, m_shallRegenerate(shallRegenerate)
	, m_shallBeRemoved(false)
	, m_hunter(hunter)
{
}

bool Food::decay(void)
{
	m_value -= config::FOOD_DECAY_STEP;
	m_shallBeRemoved |= m_value <= 0;
	return m_shallBeRemoved;
}

bool Food::hasDecayed(void) const
{
	return m_value <= 0;
}
