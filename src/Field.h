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

#include <set>
#include <memory>
#include <random>

#include "Database.h"
#include "types.h"
#include "config.h"
#include "Food.h"
#include "Bot.h"
#include "UpdateTracker.h"
#include "SpatialMap.h"
#include "BotThreadPool.h"
#include "BotUpDownThread.h"

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
		typedef std::function< void(const std::shared_ptr<Bot>&, const std::string&) > BotErrorCallback;

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
		uint32_t m_currentFrame = 0;

		BotSet  m_bots;

		BotUpDownThread m_limbo;

		std::unique_ptr<std::mt19937> m_rndGen;

		std::unique_ptr< std::normal_distribution<real_t> >       m_foodSizeDistribution;
		std::unique_ptr< std::uniform_real_distribution<real_t> > m_positionXDistribution;
		std::unique_ptr< std::uniform_real_distribution<real_t> > m_positionYDistribution;
		std::unique_ptr< std::uniform_real_distribution<real_t> > m_angleRadDistribution;
		std::unique_ptr< std::uniform_real_distribution<real_t> > m_simple0To1Distribution;

		std::unique_ptr<UpdateTracker> m_updateTracker;

		FoodMap m_foodMap;
		SegmentInfoMap m_segmentInfoMap;
		std::vector<BotKilledCallback> m_botKilledCallbacks;
		std::vector<BotErrorCallback> m_botErrorCallbacks;
		BotThreadPool m_threadPool;

		void setupRandomness(void);
		void createStaticFood(std::size_t count);

		void updateSnakeSegmentMap(void);
		void updateMaxSegmentRadius(void);

		bool isLocationOutsideSnakes(const Vector2D &pos, real_t margin = 10);
		Vector2D findFreeRandomLocation(void);

		void sendAllLogMessages(const std::shared_ptr<Bot> &b);

	public:
		Field(real_t w, real_t h, std::size_t food_parts, std::unique_ptr<UpdateTracker> update_tracker);

		/*!
		 * Create a new Bot for this field.
		 *
		 * Note that the bot will be spawned in "Limbo", that is, not really alive
		 * (on the Field), but also not completely gone. It must be initialized
		 * before it appears on the Field, which may take some time.
		 *
		 * @return shared_ptr to the new bot; non-empty initErrorMessage if initialization failed
		 */
		std::shared_ptr<Bot> newBot(std::unique_ptr<db::BotScript> data, std::string &initErrorMessage);

		/*!
		 * Handle asynchronously started and stopped bots.
		 */
		void updateLimbo(void);

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
		 * \brief process bot log messages
		 *
		 * move all pending log messages to update tracker
		 * and increase log credit for all active bots
		 */
		void processLog(void);

		/*!
		 * \brief increment current frame number and send tick message
		 */
		void tick();

		/*!
		 * Send statistics to the UpdateTracker.
		 */
		void sendStatsToStream(void);

		/*!
		 * Get the set of bots.
		 */
		const BotSet& getBots(void) const;
		std::shared_ptr<Bot> getBotByDatabaseId(int id);

		/*!
		 * Check if the given database ID is active on the Field or in Limbo.
		 */
		bool isDatabaseIdActive(int id);

		/*!
		 * Add dynamic food equally distributed in the given circle.
		 *
		 * Every Food item has values according to the config::FOOD_SIZE_MEAN and
		 * config::FOOD_SIZE_STDDEV constants.
		 *
		 * \param totalValue   Total (average) value of the food created.
		 * \param center       Center of the distribution circle.
		 * \param radius       Radius of the distribution circle.
		 * \param hunter       Pointer to the bot killing the Bot that this Food is
		 *                     spawned from.
		 */
		void createDynamicFood(real_t totalValue, const Vector2D &center, real_t radius,
				const std::shared_ptr<Bot> &hunter);

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
		void killBot(std::shared_ptr<Bot> victim, std::shared_ptr<Bot> killer);

		void addBotErrorCallback(BotErrorCallback callback);

		UpdateTracker& getUpdateTracker() { return *m_updateTracker; }

		uint32_t getCurrentFrame() { return m_currentFrame; }

		/*!
		 * Calculate the current mass on the field.
		 *
		 * Living mass is contained in snakes, dead mass is food distributed over the field.
		 */
		void calculateCurrentMass(double *living, double *dead);

		/*!
		 * Get lengths of startup and shutdown queues.
		 */
		void getLimboStats(size_t *startup_queue_len, size_t *shutdown_queue_len)
		{
			*startup_queue_len = m_limbo.getStartupQueueLen();
			*shutdown_queue_len = m_limbo.getShutdownQueueLen();
		}
};
