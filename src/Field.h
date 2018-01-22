#pragma once

#include <set>
#include <memory>

#include "Vector.h"
#include "Bot.h"

/*!
 * Representation of the playing field.
 *
 * The field is implemented as a torus surface, which means that everything
 * that leaves the area on the left comes back in and vice versa. The same is
 * true for top and bottom edge.
 */
class Field
{
	public:
		typedef std::set< std::shared_ptr<Bot> > BotSet;
		//typedef std::set< std::shared_ptr<Food> > FoodSet;

	private:
		float_t m_width;
		float_t m_height;

		BotSet  m_bots;

	public:
		Field();
		Field(float_t w, float_t h);

		/*!
		 * Create a new Bot on this field.
		 */
		void newBot(const std::string &name);

		/*!
		 * Move all bots.
		 */
		void moveAllBots(void);

		/*!
		 * Get the set of bots.
		 */
		const BotSet& getBots(void);

		/*!
		 * Wrap the coordinates of the given vector into the Fields unique area.
		 *
		 * \param v    The vector to wrap.
		 * \returns    A new vector containing the wrapped coordinates.
		 */
		Vector wrapCoords(const Vector &v);

		/*!
		 * Unwrap the coordinates of the given vector with respect to a reference
		 * vector. If the vector is less than a half field size away from the
		 * reference in the wrapped space, the result will be adjusted such that
		 * this is also the case for the plain coordinates.
		 *
		 * The vector returned by this function will be potentially outside the
		 * unique field area.
		 *
		 * \param v    The vector to unwrap.
		 * \param ref  The reference vector.
		 * \returns    A new vector containing the unwrapped coordinates.
		 */
		Vector unwrapCoords(const Vector &v, const Vector &ref);

		/*!
		 * Print a text representation of the field for debugging to stdout.
		 */
		void debugVisualization(void);
};
