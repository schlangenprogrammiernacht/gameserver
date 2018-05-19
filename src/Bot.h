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

#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "Database.h"
#include "IdentifyableObject.h"
#include "Snake.h"
#include "types.h"
#include "Stopwatch.h"

class Field;
class LuaBot;
class GlobalView;

/*!
 * A bot playing this game.
 */
class Bot : public IdentifyableObject
{
	private:
		Field *m_field;
		uint32_t m_startFrame;
		std::unique_ptr<db::BotScript> m_dbData;
		std::shared_ptr<Snake> m_snake;
		std::unique_ptr<LuaBot> m_lua_bot;
		std::vector<std::string> m_logMessages;
		real_t m_logCredit = config::LOG_INITIAL_CREDITS;

		real_t m_consumedFoodHuntedBySelf = 0;
		real_t m_consumedFoodHuntedByOthers = 0;
		real_t m_consumedNaturalFood = 0;

		Stopwatch m_swMove;

	public:
		/*!
		 * Creates a new bot identified by the given name on the given playing
		 * field.
		 */
		Bot(Field *field, uint32_t startFrame, std::unique_ptr<db::BotScript> dbData, const Vector2D &startPos, real_t startHeading);
		~Bot();

		/*!
		 * \brief init
		 * initialize the bot, e.g. parse the lua script
		 * \return true if init successful, false if failed
		 */
		bool init(std::string &initErrorMessage);

		/*!
		 * Run the bot's movement code and update the Snake’s position.
		 *
		 * \returns   The number of new segments created at the snake's head.
		 */
		std::size_t move(void);

		/*!
		 * Check collision with any bots on the Field using the Field’s GlobalView
		 * object.
		 *
		 * \returns   The Bot that this Bot collided with or NULL if no collision
		 *            occurred.
		 */
		std::shared_ptr<Bot> checkCollision(void) const;

		/*!
		 * \brief increase log credit every frame, until config::LOG_MAX_CREDITS is reached
		 */
		void increaseLogCredit();

		std::shared_ptr<Snake> getSnake(void) const { return m_snake; }
		const std::string &getName(void) const { return m_dbData->bot_name; }
		real_t getHeading() { return m_snake->getHeading(); }
		Field* getField() { return m_field; }
		int getDatabaseId() { return m_dbData->bot_id; }
		int getDatabaseVersionId() { return m_dbData->version_id; }
		LuaBot& getLuaBot() { return *m_lua_bot; }
		uint32_t getStartFrame() { return m_startFrame; }

		real_t getConsumedNaturalFood(void) { return m_consumedNaturalFood; }
		real_t getConsumedFoodHuntedByOthers(void) { return m_consumedFoodHuntedByOthers; }
		real_t getConsumedFoodHuntedBySelf(void) { return m_consumedFoodHuntedBySelf; }

		void updateConsumeStats(const Food &food);

		uint64_t getViewerKey() { return m_dbData->viewer_key; }

		bool appendLogMessage(const std::string &data, bool checkCredit);
		real_t getLogCredit() { return m_logCredit; }
		std::vector<std::string> &getLogMessages() { return m_logMessages; }
		void clearLogMessages() { m_logMessages.clear(); }

		std::vector<uint32_t> getColors();
		real_t getSightRadius() const;
		uint32_t getFace();
		uint32_t getDogTag();

		long getLastMoveTimeNs() { return m_swMove.GetThreadTimeNs(); }
		long getApiTimeNs();
};
