#pragma once

#include <set>
#include <memory>
#include <random>

#include "types.h"
#include "config.h"
#include "Food.h"
#include "Bot.h"
#include "LuaBot.h"
#include "UpdateTracker.h"
#include "SpatialMap.h"
#include "BotThreadPool.h"

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
		typedef std::function< void(std::shared_ptr<Bot>, std::shared_ptr<Bot>) > BotKilledCallback;

	public:
		struct SnakeSegmentInfo {
			const Snake::Segment &segment; //!< Pointer to the segment
			std::shared_ptr<Bot> bot; //!< The bot this segment belongs to

			SnakeSegmentInfo(const Snake::Segment &s, const std::shared_ptr<Bot> &b)
				: segment(s), bot(b) {}

			const Vector2D& pos() const { return segment.pos(); }
		};
		typedef SpatialMap<SnakeSegmentInfo, config::SPATIAL_MAP_TILES_X, config::SPATIAL_MAP_TILES_Y> SegmentInfoMap;

		typedef SpatialMap<Food, config::SPATIAL_MAP_TILES_X, config::SPATIAL_MAP_TILES_Y> FoodMap;

	private:
		const real_t m_width;
		const real_t m_height;

		real_t m_maxSegmentRadius = 0;

		BotSet  m_bots;

		std::unique_ptr<std::mt19937> m_rndGen;

		std::unique_ptr< std::normal_distribution<real_t> >       m_foodSizeDistribution;
		std::unique_ptr< std::uniform_real_distribution<real_t> > m_positionXDistribution;
		std::unique_ptr< std::uniform_real_distribution<real_t> > m_positionYDistribution;
		std::unique_ptr< std::uniform_real_distribution<real_t> > m_angleDegreesDistribution;
		std::unique_ptr< std::uniform_real_distribution<real_t> > m_angleRadDistribution;
		std::unique_ptr< std::uniform_real_distribution<real_t> > m_simple0To1Distribution;

		std::shared_ptr<UpdateTracker> m_updateTracker;

		FoodMap m_foodMap;
		SegmentInfoMap m_segmentInfoMap;
		std::vector<BotKilledCallback> m_botKilledCallbacks;
		BotThreadPool m_threadPool;

		void setupRandomness(void);
		void createStaticFood(std::size_t count);

		void updateSnakeSegmentMap(void);
		void updateMaxSegmentRadius(void);
		void onBotKilled(std::shared_ptr<Bot> victim, std::shared_ptr<Bot> killer);

	public:
		Field(real_t w, real_t h, std::size_t food_parts,
				const std::shared_ptr<UpdateTracker> &update_tracker);

		/*!
		 * Create a new Bot on this field.
		 */
		void newBot(int databaseId, const std::string &name, std::unique_ptr<LuaBot> luaBot);

		/*!
		 * Decay all food.
		 *
		 * This includes replacing static food when decayed.
		 */
		void decayFood(void);

		/*!
		 * Make all Snakes consume food in their eating range.
		 *
		 * \todo This function is searching for a better name.
		 */
		void consumeFood(void);

		/*!
		 * \brief remove decayed and consumed food
		 */
		void removeFood(void);

		/*!
		 * Move all bots and check collisions.
		 */
		void moveAllBots(void);

		/*!
		 * Get the set of bots.
		 */
		const BotSet& getBots(void) const;

		/*!
		 * Add dynamic food equally distributed in the given circle.
		 *
		 * Every Food item has values according to the config::FOOD_SIZE_MEAN and
		 * config::FOOD_SIZE_STDDEV constants.
		 *
		 * \param totalValue   Total (average) value of the food created.
		 * \param center       Center of the distribution circle.
		 * \param radius       Radius of the distribution circle.
		 */
		void createDynamicFood(real_t totalValue, const Vector2D &center, real_t radius);

		/*!
		 * Wrap the coordinates of the given vector into the Fields unique area.
		 *
		 * \param v    The vector to wrap.
		 * \returns    A new vector containing the wrapped coordinates.
		 */
		Vector2D wrapCoords(const Vector2D &v) const;

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
		Vector2D unwrapCoords(const Vector2D &v, const Vector2D &ref) const;

		Vector2D unwrapRelativeCoords(const Vector2D& relativeCoords) const;

		/*!
		 * Print a text representation of the field for debugging to stdout.
		 */
		void debugVisualization(void);

		/*!
		 * Get the size of the field.
		 */
		Vector2D getSize(void) const;

		/*!
		 * Get the maximum segment radius of any Snake on the Field.
		 */
		real_t getMaxSegmentRadius(void) const;

		FoodMap& getFoodMap() { return m_foodMap; }
		SegmentInfoMap& getSegmentInfoMap() { return m_segmentInfoMap; }

		void addBotKilledCallback(BotKilledCallback callback);

};
