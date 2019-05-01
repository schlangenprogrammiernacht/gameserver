
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

DockerBot::DockerBot(Bot &bot, std::string imageName)
	: m_bot(bot)
	, m_imageName(imageName)
	, m_swAPI("api")
	, m_shm(NULL)
	, m_listenSocket(-1)
	, m_botSocket(-1)
{
	std::regex cleanup_re(R"([^a-z0-9+_-]+)", std::regex_constants::icase);
	m_cleanName = std::regex_replace(bot.getName(), cleanup_re, "_");

	if(m_cleanName.empty()) {
		m_cleanName = "unnamed";
	}

	m_cleanName = m_cleanName.substr(0, 32);

	std::cerr << logPrefix() << "DockerBot constructed: " << bot.getName() << " [" << m_cleanName << "]" << std::endl;
}

DockerBot::~DockerBot()
{
	shutdown();
}

void DockerBot::startup()
{
	createSharedMemory();
	prepareSharedMemory();
	createSocket();
	startBot();
}

void DockerBot::shutdown(void)
{
	destroySocket();
	shutdownSubprocess();
	destroySharedMemory();
}

std::string DockerBot::logPrefix(void)
{
	return "[" + m_cleanName + "] ";
}

void DockerBot::createSharedMemory(void)
{
	std::string bot_dir = config::BOT_IPC_DIRECTORY + m_cleanName;
	std::string shm_path = bot_dir + "/shm";

	int ret = mkdir(bot_dir.c_str(), 0777);
	if(ret == -1 && (errno != EEXIST)) {
		std::cerr << logPrefix() << "mkdir() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to set up bot directory.");
	}

	int shm_fd = open(shm_path.c_str(), O_RDWR | O_CREAT, 0666);
	if(shm_fd == -1) {
		std::cerr << logPrefix() << "shm_open() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to set up shared memory.");
	}

	ret = ftruncate(shm_fd, IPC_SHARED_MEMORY_BYTES);
	if(ret == -1) {
		std::cerr << logPrefix() << "ftruncate() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to set up shared memory.");
	}

	void *shared_mem = mmap(NULL, IPC_SHARED_MEMORY_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(shared_mem == (void*)-1) {
		std::cerr << logPrefix() << "mmap() failed: " << strerror(errno) << std::endl;
		close(shm_fd);
		throw std::runtime_error("Failed to set up shared memory.");
	}

	m_shm = reinterpret_cast<struct IpcSharedMemory*>(shared_mem);
	m_shmFd = shm_fd;

	std::cerr << logPrefix() << "Set up shared memory at address " << std::hex << m_shm
		<< " with size of " << std::dec << IPC_SHARED_MEMORY_BYTES << " bytes." << std::endl;

	// set default color
	m_shm->colorCount = 1;
	m_shm->colors[0].r = 0x80;
	m_shm->colors[0].g = 0x80;
	m_shm->colors[0].b = 0x80;

	// copy persistent data
	memcpy(m_shm->persistentData, m_bot.getPreviousPersistentData().data(), IPC_PERSISTENT_MAX_BYTES);
}

void DockerBot::destroySharedMemory(void)
{
	if(!m_shm) {
		return;
	}

	int ret = munmap(m_shm, IPC_SHARED_MEMORY_BYTES);
	if(ret == -1) {
		std::cerr << logPrefix() << "munmap() failed: " << strerror(errno) << std::endl;
	}

	close(m_shmFd);

	m_shm = NULL;
}

void DockerBot::prepareSharedMemory(void)
{
	m_shm->serverConfig.snake_boost_steps               = config::SNAKE_BOOST_STEPS;
	m_shm->serverConfig.snake_turn_radius_factor        = config::SNAKE_TURN_RADIUS_FACTOR;

	m_shm->serverConfig.snake_pull_factor               = config::SNAKE_PULL_FACTOR;

	m_shm->serverConfig.snake_conversion_factor         = config::SNAKE_CONVERSION_FACTOR;

	m_shm->serverConfig.snake_segment_distance_factor   = config::SNAKE_SEGMENT_DISTANCE_FACTOR;
	m_shm->serverConfig.snake_segment_distance_exponent = config::SNAKE_SEGMENT_DISTANCE_EXPONENT;

	m_shm->serverConfig.snake_consume_range             = config::SNAKE_CONSUME_RANGE;

	m_shm->serverConfig.snake_boost_loss_factor         = config::SNAKE_BOOST_LOSS_FACTOR;
	m_shm->serverConfig.snake_survival_loss_factor      = config::SNAKE_SURVIVAL_LOSS_FACTOR;


	m_shm->serverConfig.snake_self_kill_mass_theshold   = config::SNAKE_SELF_KILL_MASS_THESHOLD;

	m_shm->serverConfig.food_decay_step                 = config::FOOD_DECAY_STEP;

	m_shm->serverConfig.log_credits_per_frame           = config::LOG_CREDITS_PER_FRAME;
	m_shm->serverConfig.log_initial_credits             = config::LOG_INITIAL_CREDITS;
	m_shm->serverConfig.log_max_credits                 = config::LOG_MAX_CREDITS;
}

void DockerBot::fillSharedMemory(void)
{
	// Step 1: self info
	m_shm->selfInfo.segment_radius = m_bot.getSnake()->getSegmentRadius();
	m_shm->selfInfo.mass           = m_bot.getSnake()->getMass();
	m_shm->selfInfo.sight_radius   = m_bot.getSightRadius();
	m_shm->selfInfo.consume_radius = m_bot.getSnake()->getConsumeRadius();

	m_shm->selfInfo.start_frame   = m_bot.getStartFrame();
	m_shm->selfInfo.current_frame = m_bot.getField()->getCurrentFrame();

	m_shm->selfInfo.speed          = config::SNAKE_DISTANCE_PER_STEP;
	m_shm->selfInfo.max_step_angle = m_bot.getSnake()->maxRotationPerStep();

	m_shm->selfInfo.consumed_natural_food          = m_bot.getConsumedNaturalFood();
	m_shm->selfInfo.consumed_food_hunted_by_self   = m_bot.getConsumedFoodHuntedBySelf();
	m_shm->selfInfo.consumed_food_hunted_by_others = m_bot.getConsumedFoodHuntedByOthers();

	// Step 2: food

	auto head_pos = m_bot.getSnake()->getHeadPosition();
	real_t heading = m_bot.getHeading();

	real_t radius = m_shm->selfInfo.sight_radius;

	real_t min_size = 0.1f; // FIXME

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
			m_shm->foodInfo[idx].val = food.getValue();
			m_shm->foodInfo[idx].dir = direction;
			m_shm->foodInfo[idx].dist = distance;

			idx++;
		}
	}

	m_shm->foodCount = idx;

	std::sort(
		std::begin(m_shm->foodInfo),
		std::begin(m_shm->foodInfo) + m_shm->foodCount,
		[](const IpcFoodInfo& a, const IpcFoodInfo& b) { return a.dist < b.dist; }
	);

	// Step 3: segments

	auto self_id = m_bot.getGUID();

	std::set< std::shared_ptr<Bot> > usedBots;

	idx = 0;
	for (auto &segmentInfo: field->getSegmentInfoMap().getRegion(head_pos, radius + m_bot.getField()->getMaxSegmentRadius()))
	{
		if(idx >= IPC_SEGMENT_MAX_COUNT) {
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
		m_shm->segmentInfo[idx].r = segmentRadius;
		m_shm->segmentInfo[idx].dir = direction;
		m_shm->segmentInfo[idx].dist = distance;
		m_shm->segmentInfo[idx].bot_id = segmentBotID;
		m_shm->segmentInfo[idx].idx = segmentInfo.segment.index;
		m_shm->segmentInfo[idx].is_self = (segmentBotID == self_id);

		usedBots.insert(segmentInfo.bot);

		idx++;
	}

	m_shm->segmentCount = idx;

	std::sort(
		std::begin(m_shm->segmentInfo),
		std::begin(m_shm->segmentInfo) + m_shm->segmentCount,
		[](const IpcSegmentInfo& a, const IpcSegmentInfo& b) { return a.dist < b.dist; }
	);

	// Step 4: bots

	idx = 0;
	for(auto bot: usedBots) {
		if(idx >= IPC_BOT_MAX_COUNT) {
			// maximum number of bots written
			break;
		}

		m_shm->botInfo[idx].bot_id = bot->getGUID();
		strncpy(m_shm->botInfo[idx].bot_name, bot->getName().c_str(), sizeof(m_shm->botInfo[idx].bot_name));

		idx++;
	}

	m_shm->botCount = idx;

	m_shm->logData[0] = '\0';
}

void DockerBot::createSocket(void)
{

	struct sockaddr_un sa;

	sa.sun_family = AF_UNIX;
	snprintf(sa.sun_path, sizeof(sa.sun_path), "%s%s/socket", config::BOT_IPC_DIRECTORY, m_cleanName.c_str());

	m_listenSockPath = sa.sun_path;

	if(unlink(m_listenSockPath.c_str()) == 0) {
		std::cerr << logPrefix() << "WARNING: removed " << m_listenSockPath << " before recreating it." << std::endl;
	}

	int s = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if(s == -1) {
		std::cerr << logPrefix() << "socket() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to set up IPC socket.");
	}

	// bind to the address
	int ret = bind(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa));
	if(ret == -1) {
		std::cerr << logPrefix() << "bind() failed: " << strerror(errno) << std::endl;
		close(s);
		throw std::runtime_error("Failed to set up IPC socket.");
	}

	// start listening
	ret = listen(s, 1);
	if(ret == -1) {
		std::cerr << logPrefix() << "listen() failed: " << strerror(errno) << std::endl;
		close(s);
		throw std::runtime_error("Failed to set up IPC socket.");
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
	// build container name
	std::ostringstream oss;
	oss << "spnbot_" << m_cleanName << "_" << m_bot.getGUID() << time(NULL);
	m_dockerContainerName = oss.str();

	int pid = fork();
	if(pid == 0) {
		// child process
		execl(config::BOT_LAUNCHER_SCRIPT, config::BOT_LAUNCHER_SCRIPT,
				m_imageName.c_str(), m_cleanName.c_str(),
				m_dockerContainerName.c_str(), (char*)NULL);

		// we only get here if execl failed
		std::cerr << logPrefix() << "execl() failed: " << strerror(errno) << std::endl;
		exit(99);
	} else if(pid == -1) {
		// error
		std::cerr << logPrefix() << "fork() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Error while starting bot process.");
	}

	// wait for the 'docker run' process to complete (container is run in background)
	int status;
	pid_t exitpid = waitpid(pid, &status, 0);

	if(WIFEXITED(status)) {
		std::cerr << logPrefix() << "'docker run' exited code " << WEXITSTATUS(status) << std::endl;
		if(WEXITSTATUS(status) == 0) {
			std::cerr << logPrefix() << "'docker run' completed successfully" << std::endl;
		} else {
			throw std::runtime_error("Error during 'docker run'.");
		}
	} else if(WIFSIGNALED(status)) {
		std::cerr << logPrefix() << "'docker run' terminated by signal " << WTERMSIG(status) << std::endl;
	} else {
		std::cerr << logPrefix() << "'docker run' terminated with unexpected exit status: " << status << std::endl;
	}

	// parent process continues
	// FIXME: replace with a bool (subprocess running)
	m_dockerPID = pid;

	// wait for a connection
	int ret = waitForReadEvent(m_listenSocket, config::BOT_CONNECT_TIMEOUT);
	if(ret <= 0) {
		shutdownSubprocess();

		if(ret == -1) {
			throw std::runtime_error("Error while waiting for bot process to connect.");
		} else {
			throw std::runtime_error("Timeout while waiting for bot process to connect.");
		}
	}

	// bot connected in time
	ret = accept(m_listenSocket, NULL, NULL);
	if(ret == -1) {
		std::cerr << "accept() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to set up IPC socket.");
	}

	m_botSocket = ret;
}

int DockerBot::shutdownSubprocess(void)
{
	if(m_dockerPID == -1) {
		// nothing to do
		return 0;
	}

	// run "docker stop" on the container
	pid_t pid = fork();
	if(pid == 0) {
		// child process
		execlp("docker", "docker", "stop", "--time=1",
				m_dockerContainerName.c_str(), (char*)NULL);

		// we only get here if execlp failed
		std::cerr << logPrefix() << "execlp() failed: " << strerror(errno) << std::endl;
		exit(99);
	} else if(pid == -1) {
		// error
		std::cerr << logPrefix() << "fork() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Error while stopping bot process.");
	}

	int status;
	pid_t exitpid = waitpid(pid, &status, 0);

	if(exitpid == -1) {
		std::cerr << logPrefix() << "waitpid() failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Error while stopping bot process.");
	}

	if(WIFEXITED(status) && (WEXITSTATUS(status) == 0)) {
		std::cerr << logPrefix() << "'docker stop' completed successfully." << std::endl;
	} else {
		std::cerr << logPrefix() << "'docker stop' terminated with unexpected status information: " << status << std::endl;
	}

	m_dockerPID = -1;

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
		std::cerr << logPrefix() << "select() failed: " << strerror(errno) << std::endl;
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
		std::cerr << logPrefix() << "select() failed: " << strerror(errno) << std::endl;
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
		std::cerr << logPrefix() << "Bot socket is not ready for write." << std::endl;
		return false;
	}

	ret = send(m_botSocket, data, length, 0);
	if(ret == -1) {
		std::cerr << logPrefix() << "send() failed: " << strerror(errno) << std::endl;
		return false;
	} else if(static_cast<size_t>(ret) != length) {
		std::cerr << logPrefix() << "Sent only " << ret << " of " << length << " bytes to bot." << std::endl;
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
		std::cerr << logPrefix() << "Read timed out." << std::endl;
		return false;
	}

	int niter = 0;
	do {
		ret = recv(m_botSocket, data, length, MSG_DONTWAIT);
		if(ret == -1) {
			if(errno != EAGAIN && errno != EWOULDBLOCK) {
				std::cerr << logPrefix() << "recv() failed: " << strerror(errno) << std::endl;
				return false;
			}
			// else end the iteration as the last packet from the socket queue was received
		} else if(static_cast<size_t>(ret) != length) {
			std::cerr << logPrefix() << "Received only " << ret << " of " << length << " bytes from bot." << std::endl;
			return false;
		}

		niter++;
	} while(ret != -1);

	if(niter > 2) {
		std::cerr << logPrefix() << "Looped " << (niter-2) << " extra times to empty the socket queue." << std::endl;
	}

	return true;
}

bool DockerBot::init(std::string &initErrorMessage)
{
	if(m_botSocket == -1) {
		initErrorMessage = "Bot is not properly prepared to run: socket not set up.";
		return false;
	} else if(m_dockerPID == -1) {
		initErrorMessage = "Bot is not properly prepared to run: docker process not running.";
		return false;
	} else if(m_shm == NULL) {
		initErrorMessage = "Bot is not properly prepared to run: shared memory not set up.";
		return false;
	}

	IpcRequest request = {REQ_INIT};

	if(!sendMessageToBot(&request, sizeof(request))) {
		initErrorMessage = "Failed to send INIT request to bot.";
		return false;
	}

	IpcResponse response;

	if(!readMessageFromBot(&response, sizeof(response), config::BOT_INIT_TIMEOUT)) {
		initErrorMessage = "Bot is not responding.";
		return false;
	}

	if(response.type != RES_OK) {
		initErrorMessage = "Bot could not initialize successfully.";
		return false;
	}

	if(m_shm->colorCount > IPC_COLOR_MAX_COUNT) {
		initErrorMessage = "Excessive number of colors returned.";
		return false;
	}

	if(m_shm->colorCount == 0) {
		m_colors.resize(1);
		m_colors[0] = 0x00EC25A2; // a nice pink for those who do not set any colors
	} else {
		m_colors.resize(m_shm->colorCount);
		for(size_t i = 0; i < m_shm->colorCount; i++) {
			uint8_t &r = m_shm->colors[i].r;
			uint8_t &g = m_shm->colors[i].g;
			uint8_t &b = m_shm->colors[i].b;

			m_colors[i] = (r << 16) | (g << 8) | (b << 0);
		}
	}

	return true;
}

bool DockerBot::step(float &directionChange, bool &boost)
{
	// all communication errors are fatal
	m_lastErrorWasFatal = true;

	if(m_botSocket == -1) {
		std::cerr << logPrefix() << "Bot is not properly prepared: socket not set up." << std::endl;
		return false;
	} else if(m_dockerPID == -1) {
		std::cerr << logPrefix() << "Bot is not properly prepared: docker process not running." << std::endl;
		return false;
	} else if(m_shm == NULL) {
		std::cerr << logPrefix() << "Bot is not properly prepared: shared memory not set up." << std::endl;
		return false;
	}

	// clear the last error message
	m_errorStream.str("");
	m_errorStream.clear();

	fillSharedMemory();

	m_swAPI.Reset();
	m_swAPI.Start();

	IpcRequest request = {REQ_STEP};

	if(!sendMessageToBot(&request, sizeof(request))) {
		m_swAPI.Stop();
		m_errorStream << "Failed to send message to bot." << std::endl;
		return false;
	}

	// Don't treat read errors as fatal errors as they may be just (unavoidable)
	// timeouts caused by system lags.
	m_lastErrorWasFatal = false;

	IpcResponse response;

	if(!readMessageFromBot(&response, sizeof(response), config::BOT_STEP_TIMEOUT)) {
		m_swAPI.Stop();
		m_errorStream << "Failed to read message from bot (timeout?)." << std::endl;
		return false;
	}

	m_swAPI.Stop();

	if(m_shm->logData[0] != '\0') {
		// log data updated
		char *startPtr = m_shm->logData;
		char *endPtr = startPtr;
		bool moreMessages = true;

		while(moreMessages && ((size_t)(endPtr - m_shm->logData) < IPC_LOG_MAX_BYTES)) {
			if(*endPtr == '\n' || *endPtr == '\0') {
				moreMessages = *endPtr == '\n';
				*endPtr = '\0';

				m_bot.appendLogMessage(startPtr, true);
				startPtr = endPtr + 1;
			}

			endPtr++;
		}
	}

	if(response.type != RES_OK) {
		m_errorStream << "Bot responded to step() with an error status: " << response.type << std::endl;

		// If the user responds with an error code, terminate immediately.
		m_lastErrorWasFatal = true;
		return false;
	}

	directionChange = response.step.deltaAngle;
	boost = response.step.boost;

	return true;
}

std::string DockerBot::getPersistentData(void)
{
	if(!m_shm) {
		return std::string();
	}

	return std::string(
			reinterpret_cast<char*>(m_shm->persistentData),
			IPC_PERSISTENT_MAX_BYTES);
}
