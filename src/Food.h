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
	public:
		enum class Type { STATIC, DYNAMIC };

		/*!
		 * Creates a new food pice at the given position and of the given value.
		 */
		Food(Type type, const Vector2D &pos, real_t value);

		void decay(void);
		bool hasDecayed(void);
		real_t getValue() const { return m_value; }

	private:
		Type m_type;
		real_t  m_value;
};
