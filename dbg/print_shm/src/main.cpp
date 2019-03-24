#include <iostream>
#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <ipc_format.h>

void print_food(const IpcFoodInfo &food)
{
	std::cout << "Location:  " << food.x << "/" << food.y << std::endl;
	std::cout << "Distance:  " << food.dist << std::endl;
	std::cout << "Direction: " << food.dir << std::endl;
	std::cout << "Value:     " << food.val << std::endl;
	std::cout << std::endl;
}

void print_segments(const IpcSegmentInfo &seg)
{
	std::cout << "Location:  " << seg.x << "/" << seg.y << std::endl;
	std::cout << "Distance:  " << seg.dist << std::endl;
	std::cout << "Direction: " << seg.dir << std::endl;
	std::cout << "Index:     " << seg.idx << std::endl;
	std::cout << "Bot ID:    " << seg.bot_id << std::endl;
	std::cout << "Is self:   " << seg.is_self << std::endl;
	std::cout << std::endl;
}

void print_bots(const IpcBotInfo &bot)
{
	std::cout << "Bot ID: " << bot.bot_id << std::endl;
	std::cout << "Name:   " << bot.bot_name << std::endl;
	std::cout << std::endl;
}

int main(int argc, char **argv)
{
	if(argc < 2 || argc > 2) {
		std::cerr << "Usage: " << argv[0] << " <shm-file>" << std::endl;
		return 1;
	}

	int fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		std::cerr << "open failed: " << strerror(errno) << std::endl;
		return 1;
	}

	struct IpcSharedMemory shm;
	if(read(fd, &shm, sizeof(shm)) == -1) {
		std::cerr << "read failed: " << strerror(errno) << std::endl;
		return 1;
	}

	close(fd);

	std::cout << "###### " << shm.foodCount << " food items:\n" << std::endl;
	for(size_t i = 0; i < shm.foodCount; i++) {
		print_food(shm.foodInfo[i]);
	}

	std::cout << "###### " << shm.segmentCount << " segments:\n" << std::endl;
	for(size_t i = 0; i < shm.segmentCount; i++) {
		print_segments(shm.segmentInfo[i]);
	}

	std::cout << "###### " << shm.botCount << " bots:\n" << std::endl;
	for(size_t i = 0; i < shm.botCount; i++) {
		print_bots(shm.botInfo[i]);
	}

}
