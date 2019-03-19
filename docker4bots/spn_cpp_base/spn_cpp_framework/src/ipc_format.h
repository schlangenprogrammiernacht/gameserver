#pragma once

#include <stdlib.h>
#include <stdint.h>

typedef float ipc_real_t;
typedef uint64_t ipc_guid_t;

/*!
 * IPC representation of a food particle.
 */
struct IpcFoodInfo {
	ipc_real_t x;      //!< Relative position X
	ipc_real_t y;      //!< Relative position Y
	ipc_real_t val;    //!< Food value
	ipc_real_t dir;    //!< Direction angle
	ipc_real_t dist;   //!< Distance
};

/*!
 * IPC representation of a bot.
 */
struct IpcBotInfo {
	ipc_guid_t bot_id;        //!< Bot ID
	char       bot_name[64];  //!< Bot name (the beginning of it, at least)
};

/*!
 * IPC representation of a snake segment.
 */
struct IpcSegmentInfo {
	ipc_real_t x;       //!< Relative position X
	ipc_real_t y;       //!< Relative position Y
	ipc_real_t r;       //!< Segment radius
	ipc_real_t dir;     //!< Direction angle
	ipc_real_t dist;    //!< Distance
	ipc_real_t idx;     //!< Segment number starting from head (idx == 0)
	ipc_guid_t bot_id;  //!< Bot ID
	bool       is_self; //!< True if this segment belongs to ones own snake
};

/*!
 * IPC representation of a color value.
 */
struct IpcColor {
	uint8_t r; //!< Red channel (0-255)
	uint8_t g; //!< Green channel (0-255)
	uint8_t b; //!< Blue channel (0-255)
};

const size_t IPC_FOOD_MAX_BYTES = 1 * 1024*1024;
const size_t IPC_FOOD_MAX_COUNT = IPC_FOOD_MAX_BYTES / sizeof(struct IpcFoodInfo);

const size_t IPC_BOT_MAX_COUNT = 1024;
const size_t IPC_BOT_MAX_BYTES = IPC_BOT_MAX_COUNT * sizeof(struct IpcBotInfo);

const size_t IPC_SEGMENT_MAX_BYTES = 1 * 1024*1024;
const size_t IPC_SEGMENT_MAX_COUNT = IPC_SEGMENT_MAX_BYTES / sizeof(struct IpcSegmentInfo);

const size_t IPC_COLOR_MAX_COUNT = 1024;
const size_t IPC_COLOR_MAX_BYTES = IPC_COLOR_MAX_COUNT * sizeof(struct IpcColor);

const size_t IPC_LOG_MAX_BYTES = 1024;

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

	char logData[IPC_LOG_MAX_BYTES];

	uint32_t faceID;
	uint32_t dogTagID;
};

const size_t IPC_SHARED_MEMORY_BYTES = sizeof(struct IpcSharedMemory);

/*
 * Communication structures.
 */

enum IpcRequestType {
	REQ_INIT, //!< Bot initialization request. Sent once after bot startup.
	REQ_STEP  //!< Bot step request. Sent every frame.
};

/*!
 * A request to the bot by the gameserver.
 *
 * Sent by the gameserver on the control socket.
 */
struct IpcRequest {
	enum IpcRequestType type;
};

enum IpcResponseType {
	RES_OK,
	RES_ERROR
};

/*!
 * Response of the Bot.
 *
 * Sent by the bot on the control socket.
 */
struct IpcResponse {
	enum IpcResponseType type;

	union {
		struct {
			ipc_real_t deltaAngle;
			bool       boost;
		} step;
	};
};
