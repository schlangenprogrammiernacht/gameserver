#pragma once

#include <memory>

// forward declarations
class Food;
class Bot;
class Field;

/*!
 * \brief Interface for a game state change tracker.
 */
class UpdateTracker
{
	public:
		virtual ~UpdateTracker() = default;

		/*!
		 * Track the event where food was consumed by a snake/bot.
		 *
		 * \param food     Pointer to the food item.
		 * \param by_bot   Pointer to the bot consuming the food.
		 */
		virtual void foodConsumed(
				const Food &food,
				const std::shared_ptr<Bot> &by_bot) = 0;

		/*!
		 * Track an event of decayed food.
		 *
		 * \param food   Pointer to the food item that has decayed.
		 */
		virtual void foodDecayed(const Food &food) = 0;

		/*!
		 * Track an event of newly created food.
		 *
		 * \param food   Pointer to the food item that has just spawned.
		 */
		virtual void foodSpawned(const Food& food) = 0;

		/*!
		 * Track an event of a new bot spawning.
		 *
		 * \param bot   Pointer to the bot that has just spawned.
		 */
		virtual void botSpawned(const std::shared_ptr<Bot> &bot) = 0;

		/*!
		 * Track an event of a bot killing another one (aka. two snakes collided).
		 *
		 * \param killer   Pointer to the bot that the victim collided with.
		 * \param victim   Pointer to the bot that died.
		 */
		virtual void botKilled(
				const std::shared_ptr<Bot> &killer,
				const std::shared_ptr<Bot> &victim) = 0;

		/*!
		 * Track a bot move event.
		 *
		 * \param bot   Pointer to the bot that moved.
		 * \param steps Steps that the bot moved forward.
		 */
		virtual void botMoved(const std::shared_ptr<Bot> &bot, std::size_t steps) = 0;

		virtual void botLogMessage(uint64_t viewerKey, const std::string& message) = 0;

		/*!
		 * Add a serialized version of the world state to the stream.
		 *
		 * \param field   Pointer to the Field containing the world state to be serialized.
		 */
		virtual void worldState(Field &field) = 0;

		virtual void tick(uint64_t frame_num) = 0;

		/*!
		 * Serialize the events added since the last reset or serialization.
		 *
		 * This also resets all internal state.
		 *
		 * \returns   A std::string containing the events in serialized form.
		 */
		virtual std::string serialize(void) = 0;

		/*!
		 * Reset the internal list of events. This is normally called once per
		 * frame.
		 */
		virtual void reset(void) = 0;
};
