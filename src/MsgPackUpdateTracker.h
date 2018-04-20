#pragma once

#include <sstream>

#include <msgpack.hpp>

#include "MsgPackProtocol.h"

#include "types.h"
#include "UpdateTracker.h"

/*!
 * \brief Implementation of UpdateTracker which serializes the events using
 * MsgPack.
 */
class MsgPackUpdateTracker : public UpdateTracker
{
	private:
		// messages that need to be filled over a frame
		std::unique_ptr<MsgPackProtocol::FoodConsumeMessage> m_foodConsumeMessage;
		std::unique_ptr<MsgPackProtocol::FoodSpawnMessage> m_foodSpawnMessage;
		std::unique_ptr<MsgPackProtocol::FoodDecayMessage> m_foodDecayMessage;
		std::unique_ptr<MsgPackProtocol::BotMoveMessage> m_botMoveMessage;

		std::ostringstream m_stream;

		void appendMessage(const msgpack::sbuffer &buf);

	public:
		MsgPackUpdateTracker();

		/* Implemented functions */
		void foodConsumed(
				const Food &food,
				const std::shared_ptr<Bot> &by_bot);

		void foodDecayed(const Food &food);

		void foodSpawned(const Food &food);

		void botSpawned(const std::shared_ptr<Bot> &bot);

		void botKilled(
				const std::shared_ptr<Bot> &killer,
				const std::shared_ptr<Bot> &victim);

		void botMoved(const std::shared_ptr<Bot> &bot, std::size_t steps);

		void gameInfo(void);

		void worldState(const std::shared_ptr<Field> &field);

		void tick(uint64_t frame_id);


		std::string serialize(void);

		void reset(void);
};
