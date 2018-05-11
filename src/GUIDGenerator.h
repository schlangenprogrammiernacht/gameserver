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

#include "types.h"

/*!
 * A singleton class generating unique IDs.
 */
class GUIDGenerator
{
	private:
		guid_t m_nextID;

		GUIDGenerator();

	public:
		guid_t newGUID(void);

		static GUIDGenerator& instance(void)
		{
			static GUIDGenerator theOneAndOnly;
			return theOneAndOnly;
		}
};
