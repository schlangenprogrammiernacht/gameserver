#include <iostream>
#include <cstring>

#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <fcntl.h>
#include <unistd.h>

#include <math.h>

#include "ipc_format.h"

#include "usercode.h"

const char IPC_SOCKET_NAME[] = "/spnshm/socket";

static std::ostream& log(void)
{
	std::cerr << "Bot: ";
	return std::cerr;
}

struct IpcSharedMemory* setup_shm(int *fd)
{
	*fd = open("/spnshm/shm", O_RDWR);
	if(*fd == -1) {
		log() << "shm_open() failed: " << strerror(errno) << std::endl;
		return NULL;
	}

	void *shared_mem = mmap(NULL, IPC_SHARED_MEMORY_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
	if(shared_mem == (void*)-1) {
		log() << "mmap() failed: " << strerror(errno) << std::endl;
		close(*fd);
		return NULL;
	}

	return reinterpret_cast<struct IpcSharedMemory*>(shared_mem);
}

void shutdown_shm(int fd, void *shm)
{
	int ret = munmap(shm, IPC_SHARED_MEMORY_BYTES);
	if(ret == -1) {
		log() << "munmap() failed: " << strerror(errno) << std::endl;
	}

	close(fd);
}

int connect_gameserver_socket(void)
{
	int s = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if(s == -1) {
		log() << "socket() failed: " << strerror(errno) << std::endl;
		return -1;
	}

	struct sockaddr_un sa;

	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, IPC_SOCKET_NAME, sizeof(sa.sun_path));

	// connect to the gameserver
	int ret = connect(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa));
	if(ret == -1) {
		log() << "connect() failed: " << strerror(errno) << std::endl;
		close(s);
		return -1;
	}

	return s;
}

int mainloop(struct IpcSharedMemory *shm, int sock_fd)
{
	bool running = true;

	Api api(shm);

	while(running) {
		// receive request
		struct IpcRequest request;
		int ret = recv(sock_fd, &request, sizeof(request), 0);
		if(ret == -1) {
			log() << "recv() failed: " << strerror(errno) << std::endl;
			return 1;
		} else if(ret == 0) {
			log() << "Gameserver disconnected." << std::endl;
			break;
		}

		bool result = false;
		switch(request.type) {
			case REQ_INIT:
				result = init(&api);
				break;

			case REQ_STEP:
				result = step(&api);
				break;

			default:
				break;
		};

		// send response
		struct IpcResponse response;
		response.type = result ? RES_OK : RES_ERROR;

		response.step.deltaAngle = api.angle;
		response.step.boost      = api.boost;

		ret = send(sock_fd, &response, sizeof(response), 0);
		if(ret == -1) {
			log() << "send() failed: " << strerror(errno) << std::endl;
			return 1;
		} else if(ret != sizeof(response)) {
			log() << "Could not send all the data :-(" << std::endl;
			return 1;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	int shm_fd;
	struct IpcSharedMemory *shm;

	/*
	// For cross-checking structure layout between different programming languages.
	printf("sizeof(IpcSelfInfo)     = %8lu\n", sizeof(IpcSelfInfo));
	printf("sizeof(IpcServerConfig) = %8lu\n", sizeof(IpcServerConfig));
	printf("sizeof(IpcFoodInfo)     = %8lu\n", sizeof(IpcFoodInfo));
	printf("sizeof(IpcBotInfo)      = %8lu\n", sizeof(IpcBotInfo));
	printf("sizeof(IpcSegmentInfo)  = %8lu\n", sizeof(IpcSegmentInfo));
	printf("sizeof(IpcColor)        = %8lu\n", sizeof(IpcColor));
	printf("sizeof(IpcSharedMemory) = %8lu\n", sizeof(IpcSharedMemory));
	printf("sizeof(IpcRequest)      = %8lu\n", sizeof(IpcRequest));
	printf("sizeof(IpcResponse)     = %8lu\n", sizeof(IpcResponse));
	printf("sizeof(bool)            = %8lu\n", sizeof(bool));
	*/

	log() << "Hello from the bot code!" << std::endl;

	log() << "Setting up shared memory..." << std::endl;

	shm = setup_shm(&shm_fd);
	if(!shm) {
		log() << "Failed to set up shared memory." << std::endl;
		return 1;
	}

	log() << "Set up shared memory at address 0x" << std::hex << shm
		<< " with size of " << std::dec << IPC_SHARED_MEMORY_BYTES << " bytes." << std::endl;

	int sock_fd = connect_gameserver_socket();
	if(sock_fd == -1) {
		log() << "Failed to connect to gameserver." << std::endl;
		shutdown_shm(shm_fd, shm);
		return 1;
	}

	int result = mainloop(shm, sock_fd);

	log() << "Shutting down shared memory..." << std::endl;

	close(sock_fd);
	shutdown_shm(shm_fd, shm);

	return result;
}
