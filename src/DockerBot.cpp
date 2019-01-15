
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

#include <iostream>
#include <regex>
#include <cstring>

#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <fcntl.h>
#include <unistd.h>

#include "Food.h"
#include "Bot.h"
#include "Food.h"
#include "Field.h"
#include "config.h"

#include "DockerBot.h"

DockerBot::DockerBot(Bot &bot, std::string botCode)
	: m_botCode(botCode)
	, m_bot(bot)
	, m_shm(NULL)
	, m_listenSocket(-1)
	, m_botSocket(-1)
{
	std::regex cleanup_re("[^a-zA-Z0-9+_-]+");
	std::regex_replace(m_cleanName.begin(), bot.getName().begin(), bot.getName.end(),
			cleanup_re, "_");

	m_cleanName = m_cleanName.substr(0, 32);
}

DockerBot::~DockerBot()
{
	destroySocket();
	destroySharedMemory();
}

void DockerBot::createSharedMemory(void)
{
	std::string shm_path = BOT_IPC_DIRECTORY + m_cleanName + "/shm";

	int shm_fd = open(shm_path.c_str(), O_RDWR, 0666);
	if(shm_fd == -1) {
		std::cerr << "shm_open() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed set up shared memory.");
	}

	void *shared_mem = mmap(NULL, IPC_SHARED_MEMORY_BYTES, PROT_READ, MAP_SHARED, *fd, 0);
	if(shared_mem == (void*)-1) {
		std::cerr << "mmap() failed: " << strerror(errno) << std::endl;
		close(shm_fd);
		throw std::runtime_error("Failed set up shared memory.");
	}

	m_shm = reinterpret_cast<struct IpcSharedMemory*>(shared_mem);
	m_shmFd = shm_fd;
}

void DockerBot::destroySharedMemory(void)
{
	if(!m_shm) {
		return;
	}

	int ret = munmap(m_shm, IPC_SHARED_MEMORY_BYTES);
	if(ret == -1) {
		std::cerr << "munmap() failed: " << strerror(errno) << std::endl;
	}

	close(m_shmFd);

	m_shm = NULL;
}

void DockerBot::createSocket(void)
{
	int s = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if(s == -1) {
		std::cerr << "socket() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed set up IPC socket.");
	}

	struct sockaddr_un sa;

	sa.sun_family = AF_UNIX;
	snprintf(sa.sun_path, sizeof(sa.sun_path), "%s%s/socket", BOT_IPC_DIRECTORY, m_cleanName.c_str());

	m_listenSockPath = sa.sun_path;

	// bind to the address
	int ret = bind(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa));
	if(ret == -1) {
		std::cerr << "bind() failed: " << strerror(errno) << std::endl;
		close(s);
		throw std::runtime_error("Failed set up IPC socket.");
	}

	// start listening
	ret = listen(s, 1);
	if(ret == -1) {
		std::cerr << "listen() failed: " << strerror(errno) << std::endl;
		close(s);
		throw std::runtime_error("Failed set up IPC socket.");
	}

	// success!
	m_listenSocket = s;
}

void DockerBot::destroySocket(void)
{
	if(m_listenSocket == -1) {
		return;
	}

	// remove the socket so it can be bound next time
	unlink(m_listenSockPath.c_str());

	if(m_botSocket != -1) {
		close(m_botSocket);
		m_botSocket = -1;
	}

	close(m_listenSocket);

	m_listenSocket = -1;
}

void DockerBot::startBot(void)
{
	// TODO: start the bot process

	// wait for a connection
	int ret = waitForReadEvent(m_listenSocket, BOT_CONNECT_TIMEOUT);
	if(ret <= 0) {
		forceBotShutdown();
	}

	if(ret == -1) {
		throw std::runtime_error("Error while waiting for bot process to connect.");
	} else {
		throw std::runtime_error("Timeout while waiting for bot process to connect.");
	}

	// bot connected in time
	int ret = accept(m_listenSocket);
	if(ret == -1) {
		std::cerr << "accept() failed: " << strerror(errno) << std::endl;
		close(s);
		throw std::runtime_error("Failed set up IPC socket.");
	}
}

int DockerBot::waitForReadEvent(int fd, real_t timeout)
{
	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(&rfds, fd);

	struct timeval tv;
	tv.tv_sec = static_cast<long>(timeout);
	tv.tv_usec = static_cast<long>(1e6 * (timeout - tv.tv_sec));

	int ret = select(fd+1, &rfds, NULL, NULL, &tv);
	if(ret == -1) {
		std::cerr << "select() failed: " << strerror(errno) << std::endl;
		close(s);
		return -1;
	}

	return ret;
}
