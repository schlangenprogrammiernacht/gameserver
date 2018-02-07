#pragma once

#include <sstream>

#include "UpdateTracker.h"

/*!
 * \brief Implementation of UpdateTracker which serializes the events using
 * MsgPack.
 */
class MsgPackUpdateTracker : public UpdateTracker
{
	private:
		std::ostringstream m_stream;

	public:
		MsgPackUpdateTracker();

		/* Implemented functions */
		void foodConsumed(
				const std::shared_ptr<Food> &food,
				const std::shared_ptr<Bot> &by_bot);

		void foodDecayed(const std::shared_ptr<Food> &food);

		void foodSpawned(const std::shared_ptr<Food> &food);

		std::string getSerializedEvents(void);

		void reset(void);
};
