/*
 * Schlangenprogrammiernacht: A programming game for GPN18.
 * Copyright (C) 2018  bytewerk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
		std::unique_ptr<MsgPackProtocol::BotMoveHeadMessage> m_botMoveHeadMessage;
		std::unique_ptr<MsgPackProtocol::BotStatsMessage> m_botStatsMessage;
		std::unique_ptr<MsgPackProtocol::BotLogMessage> m_botLogMessage;

		std::ostringstream m_stream;

		void appendMessage(const msgpack::sbuffer &buf);

	public:
		MsgPackUpdateTracker();

		/* Implemented functions */
		void foodConsumed(
				const Food &food,
				const std::shared_ptr<Bot> &by_bot) override;

		void foodDecayed(const Food &food) override;

		void foodSpawned(const Food &food) override;

		void botSpawned(const std::shared_ptr<Bot> &bot) override;

		void botKilled(
				const std::shared_ptr<Bot> &killer,
				const std::shared_ptr<Bot> &victim) override;

		void botMoved(const std::shared_ptr<Bot> &bot, std::size_t steps) override;

		void botLogMessage(uint64_t viewerKey, const std::string &message) override;

		void gameInfo(void);

		void worldState(Field &field) override;

		void tick(uint64_t frame_id) override;

		void botStats(const std::shared_ptr<Bot> &bot) override;

		std::string serialize(void) override;

		void reset(void) override;
};
