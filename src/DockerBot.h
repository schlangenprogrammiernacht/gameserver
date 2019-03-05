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
class DockerBot
{
	public:
		DockerBot(Bot &bot, std::string imageName);
		~DockerBot();

		bool buildDockerContainer(std::string &errorMessage);

		void startup(void);
		void shutdown(void);

		bool init(std::string &initErrorMessage);
		bool step(float &directionChange, bool &boost);

		const std::vector<uint32_t> &getColors() { return m_colors; }

		uint32_t getFace(void) { return m_shm->faceID; }
		uint32_t getDogTag(void) { return m_shm->dogTagID; }

		long getApiTimeNs(void) { return m_swAPI.GetThreadTimeNs(); }

	private:
		Bot&        m_bot;
		std::string m_cleanName;
		std::string m_imageName;

		Stopwatch   m_swAPI;

		IpcSharedMemory *m_shm;
		int              m_shmFd;
		int              m_dockerPID;
		std::string      m_dockerContainerName;
		int              m_listenSocket;
		std::string      m_listenSockPath;
		int              m_botSocket;

		std::vector<uint32_t> m_colors;

		void createSharedMemory(void);
		void destroySharedMemory(void);

		void fillSharedMemory(void);

		void createSocket(void);
		void destroySocket(void);

		void startBot(void);
		int  forceBotShutdown(void);
		int  cleanupSubprocess(void);

		int waitForReadEvent(int fd, real_t timeout);
		int checkIfSocketIsWriteable(int fd);

		bool sendMessageToBot(void *data, size_t length);
		bool readMessageFromBot(void *data, size_t length, real_t timeout);
};
