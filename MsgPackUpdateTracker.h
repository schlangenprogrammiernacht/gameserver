#pragma once

#include "UpdateTracker.h"

/*!
 * \brief Implementation of UpdateTracker which serializes the events using
 * MsgPack.
 */
class MsgPackUpdateTracker : public UpdateTracker
{
	public:
		/* Implemented functions */
		virtual void foodConsumed(
				const std::shared_ptr<Food> &food,
				const std::shared_ptr<Bot> &by_bot) = 0;

		virtual void foodDecayed(const std::shared_ptr<Food> &food) = 0;

		virtual void foodSpawned(const std::shared_ptr<Food> &food) = 0;

		virtual std::string getSerializedEvents(void) = 0;

		virtual void reset(void) = 0;
}
