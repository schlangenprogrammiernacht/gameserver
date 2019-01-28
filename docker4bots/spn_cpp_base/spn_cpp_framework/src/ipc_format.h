#pragma once

#include <stdlib.h>
#include <stdint.h>

typedef float ipc_real_t;
typedef uint64_t ipc_guid_t;

struct IpcFoodInfo {
	ipc_real_t x;      //!< Relative position X
	ipc_real_t y;      //!< Relative position Y
	ipc_real_t v;      //!< Food value
	ipc_real_t d;      //!< Direction angle
	ipc_real_t dist;   //!< Distance
};

struct IpcBotInfo {
	ipc_guid_t bot_id;        //!< Bot ID
	char       bot_name[64];  //!< Bot name (the beginning of it, at least)
};

struct IpcSegmentInfo {
	ipc_real_t x;       //!< Relative position X
	ipc_real_t y;       //!< Relative position Y
	ipc_real_t r;       //!< Segment radius
	ipc_real_t d;       //!< Direction angle
	ipc_real_t dist;    //!< Distance
	ipc_guid_t bot_id;  //!< Bot ID
	bool       is_self; //!< True if this segment belongs to ones own snake
};

struct IpcColor {
	uint8_t r; //!< Red channel
	uint8_t g; //!< Green channel
	uint8_t b; //!< Blue channel
};

const size_t IPC_FOOD_MAX_BYTES = 1 * 1024*1024;
const size_t IPC_FOOD_MAX_COUNT = IPC_FOOD_MAX_BYTES / sizeof(struct IpcFoodInfo);

const size_t IPC_BOT_MAX_COUNT = 1024;
const size_t IPC_BOT_MAX_BYTES = IPC_BOT_MAX_COUNT * sizeof(struct IpcBotInfo);

const size_t IPC_SEGMENT_MAX_BYTES = 1 * 1024*1024;
const size_t IPC_SEGMENT_MAX_COUNT = IPC_SEGMENT_MAX_BYTES / sizeof(struct IpcSegmentInfo);

const size_t IPC_COLOR_MAX_COUNT = 1024;
const size_t IPC_COLOR_MAX_BYTES = IPC_COLOR_MAX_COUNT * sizeof(struct IpcColor);

/*!
 * Shared memory structure.
 *
 * This structure represents the contents of the memory the bot shares with the
 * gameserver.
 */
struct IpcSharedMemory {
	uint32_t foodCount;
	struct IpcFoodInfo foodInfo[IPC_FOOD_MAX_COUNT];

	uint32_t botCount;
	struct IpcBotInfo botInfo[IPC_BOT_MAX_COUNT];

	uint32_t segmentCount;
	struct IpcSegmentInfo segmentInfo[IPC_SEGMENT_MAX_COUNT];

	uint32_t colorCount;
	struct IpcColor colors[IPC_COLOR_MAX_COUNT];

	uint32_t faceID;
	uint32_t dogTagID;
};

const size_t IPC_SHARED_MEMORY_BYTES = sizeof(struct IpcSharedMemory);

/*
 * Communication structures.
 */

enum IpcRequestType {
	REQ_INIT,
	REQ_STEP
};

struct IpcRequest {
	enum IpcRequestType type;
};

enum IpcResponseType {
	RES_OK,
	RES_ERROR
};

struct IpcResponse {
	enum IpcResponseType type;

	union {
		struct {
			ipc_real_t deltaAngle;
			bool       boost;
		} step;
	};
};
