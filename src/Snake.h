#pragma once

#include <deque>
#include <memory>

#include "Vector.h"

#include "types.h"

// forward declaration
class Field;
class Food;

/*!
 * Representation of a Snake.
 */
class Snake
{
	public:
		struct Segment {
			Vector pos;
			// more stuff like color?
		};

		typedef std::deque< std::shared_ptr<Segment> > SegmentList;

	private:
		/*!
		 * Definition: m_segments[0] is the head of the snake.
		 */
		SegmentList m_segments;

		float_t m_heading; //!< Heading of the snake in degrees (-180°..180°) from x axis

		float_t m_mass; //!< Mass (length) of the snake

		Field *m_field;

		float_t maxRotationPerStep(void);

		void ensureLengthMatchesMass(void);

	public:
		/*!
		 * Construct a unit snake (1 segment at 0/0, heading 0°).
		 */
		Snake(Field *field);

		Snake(Field *field, const Vector &start_pos, float_t start_mass,
				float_t start_heading);

		/*!
		 * Consume the given food piece.
		 */
		void consume(const std::shared_ptr<Food>& food);

		/*!
		 * Move the snake by one step if boost==false or SNAKE_BOOST_STEPS if boost==true.
		 */
		void move(float targetAngle, bool boost = false);

		/*!
		 * Get the list of segments.
		 */
		const SegmentList& getSegments(void);

		/*!
		 * Get the Snake's head position. This is a shortcut for getting the
		 * position of the first segment.
		 */
		const Vector& getHeadPosition(void);
};
