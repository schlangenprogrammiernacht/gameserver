#pragma once

#include <deque>
#include <memory>

#include "Vector.h"

#include "types.h"

class Field;

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

		Field *m_field;

		float_t maxRotationPerStep(void);
		Vector currentMovementVector(void);

		static const std::size_t BOOST_STEPS = 3;
		static const float_t DISTANCE_PER_STEP;

	public:
		/*!
		 * Construct a unit snake (1 segment at 0/0).
		 */
		Snake(Field *field);

		Snake(Field *field, const Vector &start_pos, std::size_t start_len);

		/*!
		 * Move the snake by one step if boost==false or SNAKE_BOOST_STEPS if boost==true.
		 */
		void move(float targetAngle, bool boost = false);

		/*!
		 * Get the list of segments.
		 */
		const SegmentList& getSegments(void);
};
