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

	private:
		/*!
		 * Definition: m_segments[0] is the head of the snake.
		 */
		SegmentList m_segments;

		Field *m_field;

		real_t m_mass; //!< Mass (length) of the snake

		real_t m_heading; //!< Heading of the snake in degrees (-180°..180°) from x axis

		real_t m_segmentRadius; //!< Segment radius (calculated from m_mass; cached)
		real_t m_targetSegmentDistance; //!< Distance between the segments

		real_t m_movedSinceLastSpawn = 0; //!< Distance the head has moved since the last spawned segment

		/*!
		 * Updates the length of m_segments and calculates the current m_segmentRadius
		 */
		void ensureSizeMatchesMass(void);

	public:
		/*!
		 * Construct a unit snake (1 segment at 0/0, heading 0°).
		 */
		Snake(Field *field);

		Snake(Field *field, const Vector2D &start_pos, real_t start_mass,
				real_t start_heading);

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
		std::size_t move(real_t targetAngle, bool boost);

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

		real_t getMass(void) { return m_mass; }

		real_t getConsumeRadius(void);

		real_t maxRotationPerStep(void);

};
