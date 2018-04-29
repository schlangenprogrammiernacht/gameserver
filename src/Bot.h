#pragma once

#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "Database.h"
#include "IdentifyableObject.h"
#include "Snake.h"
#include "types.h"

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
		real_t m_heading;
		std::vector<std::string> m_logMessages;
		real_t m_logCredit = config::LOG_INITIAL_CREDITS;

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
		bool init();

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
		real_t getHeading() { return m_heading; }
		Field* getField() { return m_field; }
		int getDatabaseId() { return m_dbData->bot_id; }
		int getDatabaseVersionId() { return m_dbData->version_id; }
		LuaBot& getLuaBot() { return *m_lua_bot; }
		uint32_t getStartFrame() { return m_startFrame; }
		uint64_t getViewerKey() { return m_dbData->viewer_key; }

		bool appendLogMessage(const std::string &data, bool checkCredit);
		real_t getLogCredit() { return m_logCredit; }
		std::vector<std::string> &getLogMessages() { return m_logMessages; }
		void clearLogMessages() { m_logMessages.clear(); }
};
