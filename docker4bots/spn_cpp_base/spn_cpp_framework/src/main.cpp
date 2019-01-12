#include <iostream>
#include <cstring>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

#include "ipc_format.h"

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

	std::cerr << "Set up shared memory at address 0x" << std::hex << shm << " with size of " << std::dec << IPC_SHARED_MEMORY_BYTES << " bytes." << std::endl;

	std::cerr << "Shutting down shared memory..." << std::endl;

	shutdown_shm(shm_fd, shm);

	return 0;
}
