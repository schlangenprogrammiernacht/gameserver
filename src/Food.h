#pragma once

#include <memory>

#include "IdentifyableObject.h"

#include "Snake.h"

#include "types.h"

class Field;

/*!
 * A piece of food that can be eaten by snakes.
 */
class Food : public IdentifyableObject
{
	private:
		Field   *m_field;
		Vector   m_pos;
		float_t  m_value;

	public:
		/*!
		 * Creates a new food pice at the given position and of the given value.
		 */
		Food(Field *field, const Vector &pos, float_t value);

		bool canBeEatenBy(const std::shared_ptr<Snake> &snake) const;

		const Vector& getPosition() const { return m_pos; }
		float_t       getValue() const { return m_value; }
};
