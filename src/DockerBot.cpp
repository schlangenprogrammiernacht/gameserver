
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
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <unistd.h>

#include "Food.h"
#include "Bot.h"
#include "Food.h"
#include "Field.h"
#include "config.h"

#include "DockerBot.h"

DockerBot::DockerBot(Bot &bot, std::string botCode)
	: m_bot(bot)
	, m_botCode(botCode)
	, m_shm(NULL)
	, m_listenSocket(-1)
	, m_botSocket(-1)
{
	std::regex cleanup_re("[^a-zA-Z0-9+_-]+");
	std::regex_replace(m_cleanName.begin(), bot.getName().begin(), bot.getName().end(),
			cleanup_re, "_");

	m_cleanName = m_cleanName.substr(0, 32);
}

DockerBot::~DockerBot()
{
	cleanupSubprocess();
	destroySocket();
	destroySharedMemory();
}

void DockerBot::createSharedMemory(void)
{
	std::string shm_path = config::BOT_IPC_DIRECTORY + m_cleanName + "/shm";

	int shm_fd = open(shm_path.c_str(), O_RDWR, 0666);
	if(shm_fd == -1) {
		std::cerr << "shm_open() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed set up shared memory.");
	}

	void *shared_mem = mmap(NULL, IPC_SHARED_MEMORY_BYTES, PROT_READ, MAP_SHARED, shm_fd, 0);
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

void DockerBot::fillSharedMemory(void)
{
	// Step 1: food

	auto head_pos = m_bot.getSnake()->getHeadPosition();
	real_t heading = m_bot.getHeading();

	real_t radius = m_bot.getSightRadius();

	real_t min_size = 1.0f; // FIXME

	auto field = m_bot.getField();

	size_t idx = 0;
	for (auto &food: field->getFoodMap().getRegion(head_pos, radius))
	{
		if(idx >= IPC_FOOD_MAX_COUNT) {
			// maximum amount of food written
			break;
		}

		if (food.getValue()>=min_size)
		{
			Vector2D relPos = field->unwrapRelativeCoords(food.pos() - head_pos);
			real_t direction = static_cast<real_t>(atan2(relPos.y(), relPos.x())) - heading;
			while (direction < -M_PI) { direction += 2*M_PI; }
			while (direction >  M_PI) { direction -= 2*M_PI; }
			auto distance = relPos.norm();
			if (distance>radius) { continue; }

			m_shm->foodInfo[idx].x = relPos.x();
			m_shm->foodInfo[idx].y = relPos.y();
			m_shm->foodInfo[idx].v = food.getValue();
			m_shm->foodInfo[idx].d = direction;
			m_shm->foodInfo[idx].dist = distance;

			idx++;
		}
	}

	m_shm->foodCount = idx;

	std::sort(
		std::begin(m_shm->foodInfo),
		std::end(m_shm->foodInfo),
		[](const IpcFoodInfo& a, const IpcFoodInfo& b) { return a.dist > b.dist; }
	);

	// Step 2: segments

	auto self_id = m_bot.getGUID();

	std::set< std::shared_ptr<Bot> > usedBots;

	idx = 0;
	for (auto &segmentInfo: field->getSegmentInfoMap().getRegion(head_pos, radius + m_bot.getField()->getMaxSegmentRadius()))
	{
		if(idx >= IPC_FOOD_MAX_COUNT) {
			// maximum number of segments written
			break;
		}

		guid_t segmentBotID = segmentInfo.bot->getGUID();

		real_t segmentRadius = segmentInfo.bot->getSnake()->getSegmentRadius();
		Vector2D relPos = field->unwrapRelativeCoords(segmentInfo.pos() - head_pos);
		real_t distance = relPos.norm();
		if (distance > (radius+segmentRadius)) { continue; }

		real_t direction = atan2(relPos.y(), relPos.x()) - heading;
		if (direction < -M_PI) { direction += 2*M_PI; }
		if (direction >  M_PI) { direction -= 2*M_PI; }

		m_shm->segmentInfo[idx].x = relPos.x();
		m_shm->segmentInfo[idx].y = relPos.y();
		m_shm->segmentInfo[idx].d = segmentRadius;
		m_shm->segmentInfo[idx].d = direction;
		m_shm->segmentInfo[idx].dist = distance;
		m_shm->segmentInfo[idx].bot_id = segmentBotID;
		m_shm->segmentInfo[idx].is_self = (segmentBotID == self_id);

		usedBots.insert(segmentInfo.bot);

		idx++;
	}

	m_shm->segmentCount = idx;

	std::sort(
		std::begin(m_shm->segmentInfo),
		std::end(m_shm->segmentInfo),
		[](const IpcSegmentInfo& a, const IpcSegmentInfo& b) { return a.dist > b.dist; }
	);

	// Step 3: bots

	idx = 0;
	for(auto bot: usedBots) {
		m_shm->botInfo[idx].bot_id = bot->getGUID();
		strncpy(m_shm->botInfo[idx].bot_name, bot->getName().c_str(), sizeof(m_shm->botInfo[idx].bot_name));

		idx++;
	}

	m_shm->botCount = idx;
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
	snprintf(sa.sun_path, sizeof(sa.sun_path), "%s%s/socket", config::BOT_IPC_DIRECTORY, m_cleanName.c_str());

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
	int pid = fork();
	if(pid == 0) {
		// child process
		execl(config::BOT_LAUNCHER_SCRIPT, config::BOT_LAUNCHER_SCRIPT,
				m_cleanName.c_str(), (char*)NULL);

		// we only get here if execl failed
		std::cerr << "execl() failed: " << strerror(errno) << std::endl;
		exit(99);
	} else if(pid == -1) {
		// error
		std::cerr << "fork() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Error while starting bot process.");
	}

	// parent process continues
	m_dockerPID = pid;

	// wait for a connection
	int ret = waitForReadEvent(m_listenSocket, config::BOT_CONNECT_TIMEOUT);
	if(ret <= 0) {
		forceBotShutdown();
	}

	if(ret == -1) {
		throw std::runtime_error("Error while waiting for bot process to connect.");
	} else {
		throw std::runtime_error("Timeout while waiting for bot process to connect.");
	}

	// bot connected in time
	ret = accept(m_listenSocket, NULL, NULL);
	if(ret == -1) {
		std::cerr << "accept() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed set up IPC socket.");
	}
}

int DockerBot::forceBotShutdown(void)
{
	pid_t pid = fork();
	if(pid == 0) {
		// child process
		execl("docker", "docker", "stop", "--time=3",
				("spnbot:" + m_cleanName).c_str(), (char*)NULL);

		// we only get here if execl failed
		std::cerr << "execl() failed: " << strerror(errno) << std::endl;
		exit(99);
	} else if(pid == -1) {
		// error
		std::cerr << "fork() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Error while stopping bot process.");
	}

	int status;
	int exitpid = waitpid(pid, &status, 0);

	if(exitpid == -1) {
		std::cerr << "waitpid() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Error while stopping bot process.");
	}

	if(WIFEXITED(status) && (WEXITSTATUS(status) == 0)) {
		std::cerr << "Bot " << m_cleanName << " shut down successfully forced." << std::endl;
		cleanupSubprocess();
		return 0;
	} else {
		std::cerr << "'docker stop' terminated with unexpected status information: " << status << std::endl;
		return -1;
	}

	return 0;
}

int DockerBot::cleanupSubprocess(void)
{
	// this function expects that the bot was shut down before, but if it’s still
	// running, the shutdown is forced.

	if(m_dockerPID == -1) {
		// nothing to do
		return 0;
	}

	int status;
	int exitpid = waitpid(m_dockerPID, &status, WNOHANG);

	if(exitpid == 0) {
		// bot still running -> force shutdown
		forceBotShutdown();
		// wait for exit (blocking this time)
		exitpid = waitpid(m_dockerPID, &status, 0);
	}

	if(WIFEXITED(status)) {
		std::cerr << "Bot exited normally with code " << WEXITSTATUS(status) << std::endl;
	} else if(WIFSIGNALED(status)) {
		std::cerr << "Bot terminated by signal " << WSTOPSIG(status) << std::endl;
	} else {
		std::cerr << "Bot terminated with unexpected exit status: " << status << std::endl;
	}

	return 0;
}

int DockerBot::waitForReadEvent(int fd, real_t timeout)
{
	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	struct timeval tv;
	tv.tv_sec = static_cast<long>(timeout);
	tv.tv_usec = static_cast<long>(1e6 * (timeout - tv.tv_sec));

	int ret = select(fd+1, &rfds, NULL, NULL, &tv);
	if(ret == -1) {
		std::cerr << "select() failed: " << strerror(errno) << std::endl;
		return -1;
	}

	return ret;
}

int DockerBot::checkIfSocketIsWriteable(int fd)
{
	fd_set wfds;

	FD_ZERO(&wfds);
	FD_SET(fd, &wfds);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	int ret = select(fd+1, NULL, &wfds, NULL, &tv);
	if(ret == -1) {
		std::cerr << "select() failed: " << strerror(errno) << std::endl;
		return -1;
	}

	return ret;
}

bool DockerBot::sendMessageToBot(void *data, size_t length)
{
	int ret = checkIfSocketIsWriteable(m_botSocket);
	if(ret == -1) {
		return false;
	} else if(ret == 0) {
		std::cerr << "Bot socket is not ready for write." << std::endl;
		return false;
	}

	ret = send(m_botSocket, data, length, 0);
	if(ret == -1) {
		std::cerr << "send() failed: " << strerror(errno) << std::endl;
		return false;
	} else if(ret != length) {
		std::cerr << "Sent only " << ret << " of " << length << " bytes to bot." << std::endl;
		return false;
	}

	return true;
}

bool DockerBot::readMessageFromBot(void *data, size_t length, real_t timeout)
{
	int ret = waitForReadEvent(m_botSocket, timeout);
	if(ret == -1) {
		return false;
	} else if(ret == 0) {
		std::cerr << "Read timed out." << std::endl;
		return false;
	}

	ret = recv(m_botSocket, data, length, 0);
	if(ret == -1) {
		std::cerr << "recv() failed: " << strerror(errno) << std::endl;
		return false;
	} else if(ret != length) {
		std::cerr << "Received only " << ret << " of " << length << " bytes from bot." << std::endl;
		return false;
	}

	return true;
}

bool DockerBot::init(std::string &initErrorMessage)
{
	if(m_botSocket == -1 || m_dockerPID == -1 || m_shm == NULL) {
		std::cerr << "Bot is not initialized properly." << std::endl;
		return false;
	}

	IpcRequest request = {REQ_INIT};

	if(!sendMessageToBot(&request, sizeof(request))) {
		return false;
	}

	IpcResponse response;

	if(!readMessageFromBot(&response, sizeof(response), config::BOT_INIT_TIMEOUT)) {
		return false;
	}

	// TODO: read colors here

	return true;
}

bool DockerBot::step(float &directionChange, bool &boost)
{
	if(m_botSocket == -1 || m_dockerPID == -1 || m_shm == NULL) {
		std::cerr << "Bot is not initialized properly." << std::endl;
		return false;
	}

	fillSharedMemory();

	IpcRequest request = {REQ_STEP};

	if(!sendMessageToBot(&request, sizeof(request))) {
		return false;
	}

	IpcResponse response;

	if(!readMessageFromBot(&response, sizeof(response), config::BOT_STEP_TIMEOUT)) {
		return false;
	}

	if(response.type != RES_OK) {
		std::cerr << "Bot sent an error status: " << response.type << std::endl;
		return false;
	}

	// TODO: read shared memory here

	return true;
}
