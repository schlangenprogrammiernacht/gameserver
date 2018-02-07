#pragma once

#include <memory>

// forward declarations
class Food;
class Bot;

/*!
 * \brief Interface for a game state change tracker.
 */
class UpdateTracker
{
	public:
		/*!
		 * Track the event where food was consumed by a snake/bot.
		 *
		 * \param food     Pointer to the food item.
		 * \param by_bot   Pointer to the bot consuming the food.
		 */
		virtual void foodConsumed(
				const std::shared_ptr<Food> &food,
				const std::shared_ptr<Bot> &by_bot) = 0;

		/*!
		 * Track an event of decayed food.
		 *
		 * \param food   Pointer to the food item that has decayed.
		 */
		virtual void foodDecayed(const std::shared_ptr<Food> &food) = 0;

		/*!
		 * Track an event of newly created food.
		 *
		 * \param food   Pointer to the food item that has just spawned.
		 */
		virtual void foodSpawned(const std::shared_ptr<Food> &food) = 0;

		/*!
		 * Get the serialized string of events added since the last reset.
		 *
		 * \returns   A std::string containing the events in serialized form.
		 */
		virtual std::string getSerializedEvents(void) = 0;

		/*!
		 * Reset the internal list of events. This is normally called once per
		 * frame.
		 */
		virtual void reset(void) = 0;
};
