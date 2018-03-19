#pragma once

#include <memory>
#include <string>

#include "IdentifyableObject.h"

#include "Snake.h"

#include "types.h"

class Field;

/*!
 * A bot playing this game.
 */
class Bot : public IdentifyableObject
{
	private:
		std::string             m_name;
		Field                  *m_field;
		std::shared_ptr<Snake>  m_snake;

		float_t                 m_heading;

		size_t m_moveCounter;

	public:
		/*!
		 * Creates a new bot identified by the given name on the given playing
		 * field.
		 */
		Bot(Field *field, const std::string &name, const Vector2D &startPos, float_t startHeading);

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

		std::shared_ptr<Snake> getSnake(void) const;

		const std::string &getName(void) const;
};
