#include <iostream>
#include <iomanip>
#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <ipc_format.h>

void show_offset(const char *membername, const char *structname, size_t offset)
{
	std::cout << "Offset of " << std::setfill(' ') << std::setw(32) << std::left << membername
		<< " in " << structname << ": "
		<< std::right << std::dec << std::setw(10) << offset << " bytes [0x"
		<< std::hex << std::setfill('0') << std::setw(8) << offset << "]" << std::endl;
}

#define SHOW_OFFSET(type, member) \
	show_offset(#member, #type, offsetof(type, member))

int main(void)
{
	struct IpcSharedMemory shm;

	std::cout << "\n### Top-level Shared Memory Structure ###\n" << std::endl;
	std::cout << "Total Structure size: " << std::dec << sizeof(shm) << " byte.\n" << std::endl;

	SHOW_OFFSET(IpcSharedMemory, serverConfig);
	SHOW_OFFSET(IpcSharedMemory, selfInfo);
	SHOW_OFFSET(IpcSharedMemory, foodCount);
	SHOW_OFFSET(IpcSharedMemory, foodInfo);
	SHOW_OFFSET(IpcSharedMemory, botCount);
	SHOW_OFFSET(IpcSharedMemory, botInfo);
	SHOW_OFFSET(IpcSharedMemory, segmentCount);
	SHOW_OFFSET(IpcSharedMemory, segmentInfo);
	SHOW_OFFSET(IpcSharedMemory, colorCount);
	SHOW_OFFSET(IpcSharedMemory, colors);
	SHOW_OFFSET(IpcSharedMemory, logData);
	SHOW_OFFSET(IpcSharedMemory, faceID);
	SHOW_OFFSET(IpcSharedMemory, dogTagID);
	SHOW_OFFSET(IpcSharedMemory, persistentData);

	std::cout << "\n### IpcServerConfig ###\n" << std::endl;
	std::cout << "Total Structure size: " << std::dec << sizeof(struct IpcServerConfig) << " byte.\n" << std::endl;

	SHOW_OFFSET(IpcServerConfig, snake_boost_steps);
	SHOW_OFFSET(IpcServerConfig, snake_turn_radius_factor);
	SHOW_OFFSET(IpcServerConfig, snake_pull_factor);
	SHOW_OFFSET(IpcServerConfig, snake_conversion_factor);
	SHOW_OFFSET(IpcServerConfig, snake_segment_distance_factor);
	SHOW_OFFSET(IpcServerConfig, snake_segment_distance_exponent);
	SHOW_OFFSET(IpcServerConfig, snake_consume_range);
	SHOW_OFFSET(IpcServerConfig, snake_boost_loss_factor);
	SHOW_OFFSET(IpcServerConfig, snake_survival_loss_factor);
	SHOW_OFFSET(IpcServerConfig, snake_self_kill_mass_threshold);
	SHOW_OFFSET(IpcServerConfig, food_decay_step);
	SHOW_OFFSET(IpcServerConfig, log_credits_per_frame);
	SHOW_OFFSET(IpcServerConfig, log_initial_credits);
	SHOW_OFFSET(IpcServerConfig, log_max_credits);

	std::cout << "\n### IpcSelfInfo ###\n" << std::endl;
	std::cout << "Total Structure size: " << std::dec << sizeof(struct IpcSelfInfo) << " byte.\n" << std::endl;

	SHOW_OFFSET(IpcSelfInfo, segment_radius);
	SHOW_OFFSET(IpcSelfInfo, mass);
	SHOW_OFFSET(IpcSelfInfo, sight_radius);
	SHOW_OFFSET(IpcSelfInfo, consume_radius);
	SHOW_OFFSET(IpcSelfInfo, start_frame);
	SHOW_OFFSET(IpcSelfInfo, current_frame);
	SHOW_OFFSET(IpcSelfInfo, speed);
	SHOW_OFFSET(IpcSelfInfo, max_step_angle);
	SHOW_OFFSET(IpcSelfInfo, consumed_natural_food);
	SHOW_OFFSET(IpcSelfInfo, consumed_food_hunted_by_self);
	SHOW_OFFSET(IpcSelfInfo, consumed_food_hunted_by_others);

	std::cout << "\n### IpcFoodInfo ###\n" << std::endl;
	std::cout << "Total Structure size: " << std::dec << sizeof(struct IpcFoodInfo) << " byte.\n" << std::endl;

	size_t delta = (uint8_t*)&shm.foodInfo[1] - (uint8_t*)&shm.foodInfo[0];

	std::cout << "Offset from one food entry to the next: " << std::dec << delta
		<< " bytes [0x" << std::hex << std::setfill('0') << std::setw(8) << delta << "]\n" << std::endl;

	SHOW_OFFSET(IpcFoodInfo, x);
	SHOW_OFFSET(IpcFoodInfo, y);
	SHOW_OFFSET(IpcFoodInfo, val);
	SHOW_OFFSET(IpcFoodInfo, dir);
	SHOW_OFFSET(IpcFoodInfo, dist);

	std::cout << "\n### IpcBotInfo ###\n" << std::endl;
	std::cout << "Total Structure size: " << std::dec << sizeof(struct IpcBotInfo) << " byte.\n" << std::endl;

	delta = (uint8_t*)&shm.botInfo[1] - (uint8_t*)&shm.botInfo[0];

	std::cout << "Offset from one food entry to the next: " << std::dec << delta
		<< " bytes [0x" << std::hex << std::setfill('0') << std::setw(8) << delta << "]\n" << std::endl;

	SHOW_OFFSET(IpcBotInfo, bot_id);
	SHOW_OFFSET(IpcBotInfo, bot_name);

	std::cout << "\n### IpcSegmentInfo ###\n" << std::endl;
	std::cout << "Total Structure size: " << std::dec << sizeof(struct IpcSegmentInfo) << " byte.\n" << std::endl;

	delta = (uint8_t*)&shm.segmentInfo[1] - (uint8_t*)&shm.segmentInfo[0];

	std::cout << "Offset from one segment entry to the next: " << std::dec << delta
		<< " bytes [0x" << std::hex << std::setfill('0') << std::setw(8) << delta << "]\n" << std::endl;

	SHOW_OFFSET(IpcSegmentInfo, x);
	SHOW_OFFSET(IpcSegmentInfo, y);
	SHOW_OFFSET(IpcSegmentInfo, r);
	SHOW_OFFSET(IpcSegmentInfo, dir);
	SHOW_OFFSET(IpcSegmentInfo, dist);
	SHOW_OFFSET(IpcSegmentInfo, idx);
	SHOW_OFFSET(IpcSegmentInfo, bot_id);
	SHOW_OFFSET(IpcSegmentInfo, is_self);

	std::cout << "\n### IpcColor ###\n" << std::endl;
	std::cout << "Total Structure size: " << std::dec << sizeof(struct IpcColor) << " byte.\n" << std::endl;

	delta = (uint8_t*)&shm.colors[1] - (uint8_t*)&shm.colors[0];

	std::cout << "Offset from one segment entry to the next: " << std::dec << delta
		<< " bytes [0x" << std::hex << std::setfill('0') << std::setw(8) << delta << "]\n" << std::endl;

	SHOW_OFFSET(IpcColor, r);
	SHOW_OFFSET(IpcColor, g);
	SHOW_OFFSET(IpcColor, b);
}
