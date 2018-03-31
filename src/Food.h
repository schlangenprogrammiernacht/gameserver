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
		/*!
		 * Creates a new food pice at the given position and of the given value.
		 */
		Food(bool shallRegenerate, const Vector2D &pos, real_t value);

		bool decay(void);
		bool hasDecayed(void) const;
		real_t getValue() const { return m_value; }
		bool shallRegenerate() const { return m_shallRegenerate; }
		bool shallBeRemoved() const { return m_shallBeRemoved; }
		void markForRemove() { m_shallBeRemoved = true; }

	private:
		real_t  m_value;
		bool m_shallRegenerate;
		bool m_shallBeRemoved;
};
