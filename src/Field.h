#pragma once

#include <set>
#include <memory>
#include <random>

#include "UpdateTracker.h"
#include "GlobalView.h"

#include "Vector.h"
#include "Food.h"
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
		typedef std::set< std::shared_ptr<Food> > FoodSet;

	private:
		float_t m_width;
		float_t m_height;

		BotSet  m_bots;
		FoodSet m_staticFood; //!< Food placed randomly in the field.
		FoodSet m_dynamicFood; //!< Food generated dynamically by dying snakes.

		std::unique_ptr<std::mt19937> m_rndGen;

		std::unique_ptr< std::normal_distribution<float_t> >       m_foodSizeDistribution;
		std::unique_ptr< std::uniform_real_distribution<float_t> > m_positionXDistribution;
		std::unique_ptr< std::uniform_real_distribution<float_t> > m_positionYDistribution;
		std::unique_ptr< std::uniform_real_distribution<float_t> > m_headingDistribution;

		std::shared_ptr<UpdateTracker> m_updateTracker;

		GlobalView m_globalView;

		void setupRandomness(void);
		void createStaticFood(std::size_t count);

		void updateGlobalView(void);

	public:
		Field(float_t w, float_t h, std::size_t food_parts,
				const std::shared_ptr<UpdateTracker> &update_tracker);

		/*!
		 * Create a new Bot on this field.
		 */
		void newBot(const std::string &name);

		/*!
		 * Update all food pieces.
		 *
		 * This includes decaying them and replacing them when decayed.
		 */
		void updateFood(void);

		/*!
		 * Make all Snakes consume food in their eating range.
		 *
		 * \todo This function is searching for a better name.
		 */
		void consumeFood(void);

		/*!
		 * Move all bots.
		 */
		void moveAllBots(void);

		/*!
		 * Get the set of bots.
		 */
		const BotSet& getBots(void) const;

		/*!
		 * Get the set of static food.
		 */
		const FoodSet& getStaticFood(void) const;

		/*!
		 * Get the set of dynamic food.
		 */
		const FoodSet& getDynamicFood(void) const;

		/*!
		 * Wrap the coordinates of the given vector into the Fields unique area.
		 *
		 * \param v    The vector to wrap.
		 * \returns    A new vector containing the wrapped coordinates.
		 */
		Vector wrapCoords(const Vector &v) const;

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
		Vector unwrapCoords(const Vector &v, const Vector &ref) const;

		/*!
		 * Print a text representation of the field for debugging to stdout.
		 */
		void debugVisualization(void);

		/*!
		 * Get the size of the field.
		 */
		Vector getSize(void) const;
};
