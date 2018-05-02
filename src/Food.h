#pragma once

#include <memory>

#include "IdentifyableObject.h"

#include "types.h"
#include "PositionObject.h"

class Field;
class Bot;

/*!
 * A piece of food that can be eaten by snakes.
 */
class Food : public IdentifyableObject, public PositionObject
{
	public:
		/*!
		 * Creates a new food pice at the given position and of the given value.
		 */
		Food(bool shallRegenerate, const Vector2D &pos, real_t value,
				const std::shared_ptr<Bot> &hunter = nullptr);

		bool decay(void);
		bool hasDecayed(void) const;
		real_t getValue() const { return m_value; }
		bool shallRegenerate() const { return m_shallRegenerate; }
		bool shallBeRemoved() const { return m_shallBeRemoved; }
		void markForRemove() { m_shallBeRemoved = true; }

		/*!
		 * Get the hunting Bot causing this Food to be created.
		 *
		 * This will return a nullptr for Food which spawned "naturally" or from boosting Bots.
		 *
		 * \returns A shared pointer to the hunting Bot.
		 */
		std::shared_ptr<Bot> getHunter(void) const { return m_hunter; }

	private:
		real_t  m_value;
		bool m_shallRegenerate;
		bool m_shallBeRemoved;

		std::shared_ptr<Bot> m_hunter;
};
