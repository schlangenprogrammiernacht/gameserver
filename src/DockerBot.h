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

#include <ipc_format.h>

#include "config.h"
#include "Stopwatch.h"

class Bot;
class LuaBot
{
	public:
		LuaBot(Bot &bot, std::string botCode);

		bool buildDockerContainer(std::string &errorMessage);

		bool init(std::string &initErrorMessage);
		bool step(float &directionChange, bool &boost);

		std::vector<uint32_t> &getColors();

		uint32_t getFace(void);
		uint32_t getDogTag(void);

	private:
		Bot&        m_bot;
		std::string m_cleanName;

		IpcSharedMemory *m_shm;
		int              m_shmFd;
		int              m_listenSocket;
		int              m_botSocket;

		void createSharedMemory(void);
		void fillSharedMemory(void);

		void startBot(void);

		int waitForReadEvent(int fd, real_t timeout);
};
