#pragma once

#include <memory>
#include <string>

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
		int m_databaseId;
		std::string m_name;
		Field *m_field;
		std::shared_ptr<Snake> m_snake;
		std::unique_ptr<LuaBot> m_lua_bot;
		real_t m_heading;
		size_t m_moveCounter;

	public:
		/*!
		 * Creates a new bot identified by the given name on the given playing
		 * field.
		 */
		Bot(Field *field, std::unique_ptr<LuaBot> luaBot, int databaseId, const std::string &name, const Vector2D &startPos, real_t startHeading);
		~Bot();

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

		std::shared_ptr<Snake> getSnake(void) const { return m_snake; }

		const std::string &getName(void) const { return m_name; }

		real_t getHeading() { return m_heading; }

		Field* getField() { return m_field; }

		int getDatabaseId() { return m_databaseId; }
		LuaBot& getLuaBot() { return *m_lua_bot; }
};
