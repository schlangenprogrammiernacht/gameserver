#pragma once

#include <memory>

#include "IdentifyableObject.h"

#include "Snake.h"

#include "types.h"
#include "PositionObject.h"

class Field;

/*!
 * A piece of food that can be eaten by snakes.
 */
class Food : public IdentifyableObject, public PositionObject
{
	private:
		Field   *m_field;
		float_t  m_value;

	public:
		/*!
		 * Creates a new food pice at the given position and of the given value.
		 */
		Food(Field *field, const Vector2D &pos, float_t value);

		void decay(void);
		bool hasDecayed(void);
		float_t getValue() const { return m_value; }
};
