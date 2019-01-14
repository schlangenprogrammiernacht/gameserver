#include <iostream>
#include <cstring>

#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <fcntl.h>
#include <unistd.h>

#include "ipc_format.h"

const char IPC_SOCKET_NAME[] = "/spnshm/socket";

struct IpcSharedMemory* setup_shm(int *fd)
{
	*fd = open("/spnshm/shm", O_RDWR);
	if(*fd == -1) {
		std::cerr << "shm_open() failed: " << strerror(errno) << std::endl;
		return NULL;
	}

	void *shared_mem = mmap(NULL, IPC_SHARED_MEMORY_BYTES, PROT_READ, MAP_SHARED, *fd, 0);
	if(shared_mem == (void*)-1) {
		std::cerr << "mmap() failed: " << strerror(errno) << std::endl;
		close(*fd);
		return NULL;
	}

	return reinterpret_cast<struct IpcSharedMemory*>(shared_mem);
}

void shutdown_shm(int fd, void *shm)
{
	int ret = munmap(shm, IPC_SHARED_MEMORY_BYTES);
	if(ret == -1) {
		std::cerr << "munmap() failed: " << strerror(errno) << std::endl;
	}

	close(fd);
}

int connect_gameserver_socket(void)
{
	int s = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if(s == -1) {
		std::cerr << "socket() failed: " << strerror(errno) << std::endl;
		return -1;
	}

	struct sockaddr_un sa;

	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, IPC_SOCKET_NAME, sizeof(sa.sun_path));

	// connect to the gameserver
	int ret = connect(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa));
	if(ret == -1) {
		std::cerr << "connect() failed: " << strerror(errno) << std::endl;
		close(s);
		return -1;
	}

	return s;
}

int mainloop(struct IpcSharedMemory *shm, int sock_fd)
{
	bool running = true;

	while(running) {
		uint8_t message[1024];
		int ret = recv(sock_fd, message, sizeof(message), 0);
		if(ret == -1) {
			std::cerr << "recv() failed: " << strerror(errno) << std::endl;
			return 1;
		} else if(ret == 0) {
			std::cerr << "Gameserver disconnected." << std::endl;
			break;
		}

		message[ret] = '\0';
		std::cerr << "Received message:\n" << message << std::endl;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int shm_fd;
	struct IpcSharedMemory *shm;

	std::cerr << "Hello from the bot code!" << std::endl;

	std::cerr << "Setting up shared memory..." << std::endl;

	shm = setup_shm(&shm_fd);
	if(!shm) {
		std::cerr << "Failed to set up shared memory." << std::endl;
		return 1;
	}

	std::cerr << "Set up shared memory at address 0x" << std::hex << shm
		<< " with size of " << std::dec << IPC_SHARED_MEMORY_BYTES << " bytes." << std::endl;

	int sock_fd = connect_gameserver_socket();
	if(sock_fd == -1) {
		std::cerr << "Failed to connect to gameserver." << std::endl;
		shutdown_shm(shm_fd, shm);
		return 1;
	}

	int result = mainloop(shm, sock_fd);

	std::cerr << "Shutting down shared memory..." << std::endl;

	close(sock_fd);
	shutdown_shm(shm_fd, shm);

	return result;
}
