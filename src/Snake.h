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

#include <deque>
#include <memory>

#include "types.h"
#include "PositionObject.h"

// forward declaration
class Field;
class Food;
class Bot;

/*!
 * Representation of a Snake.
 */
class Snake
{
	public:
		class Segment : public PositionObject
		{
			public:
				Segment() : PositionObject(Vector2D {0,0}) {}
				Segment(const Vector2D &position) : PositionObject(position) {}
				Segment(const Segment &other) : PositionObject(other.pos()) {}
				const Segment& operator = (const Segment &other) { setPos(other.pos()); return *this; }
				// more stuff like color?
		};

		typedef std::deque< Segment > SegmentList;
		typedef std::vector< Vector2D > PositionList;

	private:
		/*!
		 * Definition: m_segments[0] is the head of the snake.
		 */
		SegmentList m_segments;

		PositionList m_headPositionsDuringLastMove;

		Field *m_field;

		real_t m_mass; //!< Mass (length) of the snake

		real_t m_heading; //!< Heading of the snake in radians from x axis

		real_t m_segmentRadius; //!< Segment radius (calculated from m_mass; cached)
		real_t m_targetSegmentDistance; //!< Distance between the segments

		real_t m_movedSinceLastSpawn = 0; //!< Distance the head has moved since the last spawned segment

		real_t m_boostedLastMove = false; //!< Track if the snake boosted during the last move

		real_t m_foodToDrop = 0;
	public:
		/*!
		 * Construct a unit snake (1 segment at 0/0, heading 0°).
		 */
		Snake(Field *field);

		Snake(Field *field, const Vector2D &start_pos, real_t start_mass,
				real_t start_heading);

		/*!
		 * Updates the length of m_segments and calculates the current m_segmentRadius
		 */
		void ensureSizeMatchesMass(void);

		/*!
		 * Consume the given food piece.
		 */
		void consume(const Food &food);

		/*!
		 * Move the snake by one step if boost==false or SNAKE_BOOST_STEPS if boost==true.
		 *
		 * \param targetAngle    The target angle the snake should head to (in degrees)
		 * \param boost          Whether to apply boost
		 * \returns              The number of segments created during the operation.
		 */
		std::size_t move(real_t deltaAngle, bool boost);

		/*!
		 * Get the list of segments.
		 */
		const SegmentList& getSegments(void) const;

		/*!
		 * Get the Snake's head position. This is a shortcut for getting the
		 * position of the first segment.
		 */
		const Vector2D& getHeadPosition(void) const;

		/*!
		 * Get the current segment radius.
		 */
		real_t getSegmentRadius(void) const;

		/*!
		 * Check if this Snake can consume the given Food.
		 */
		bool canConsume(const Food &food);

		bool tryConsume(const Food &food)
		{
			if (!canConsume(food))
			{
				return false;
			}
			consume(food);
			return true;
		}

		/*!
		 * Convert this Snake to Food. This is normally the last action before the
		 * Snake is removed from the Field.
		 */
		void convertToFood(const std::shared_ptr<Bot> &hunter) const;

		/*!
		 * Drop food at the end of the Snake. Primarily used for mass loss during boost.
		 *
		 * The Snake's mass is reduced by the given amount. The dropped mass is the
		 * given amount multiplied with SNAKE_CONVERSION_FACTOR.
		 */
		void dropFood(float_t value);

		real_t getHeading(void) { return m_heading; }

		real_t getMass(void) { return m_mass; }

		real_t getConsumeRadius(void);

		real_t maxRotationPerStep(void);

		bool boostedLastMove(void) const { return m_boostedLastMove; }

		/*!
		 * Get a list of head positions that were used during the last call to move().
		 */
		const PositionList& getHeadPositionsDuringLastMove(void) const { return m_headPositionsDuringLastMove; };
};
