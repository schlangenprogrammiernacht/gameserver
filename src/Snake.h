#pragma once

#include <deque>
#include <memory>

#include "Vector.h"

#include "types.h"

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

	private:
		/*!
		 * Definition: m_segments[0] is the head of the snake.
		 */
		std::deque< std::shared_ptr<Segment> > m_segments;

		float_t maxRotationPerStep(void);
		Vector currentMovementVector(void);

		static const std::size_t BOOST_STEPS = 3;
		static const float_t DISTANCE_PER_STEP;

	public:
		/*!
		 * Construct a unit snake (1 segment at 0/0).
		 */
		Snake();

		Snake(const Vector &start_pos, std::size_t start_len);

		/*!
		 * Move the snake by one step if boost==false or SNAKE_BOOST_STEPS if boost==true.
		 */
		void move(float targetAngle, bool boost = false);
};
