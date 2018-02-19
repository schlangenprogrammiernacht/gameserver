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

	public:
		/*!
		 * Creates a new bot identified by the given name on the given playing
		 * field.
		 */
		Bot(Field *field, const std::string &name, const Vector &startPos, float_t startHeading);

		/*!
		 * Run the bot's movement code and update the Snake’s position.
		 *
		 * \returns   The number of new segments created at the snake's head.
		 */
		std::size_t move(void);

		std::shared_ptr<Snake> getSnake(void);
};
