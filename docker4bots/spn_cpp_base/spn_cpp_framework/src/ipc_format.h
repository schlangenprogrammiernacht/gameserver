#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

extern "C" {

#define ALIGNED __attribute__((aligned(4)))

typedef float ipc_real_t;
typedef uint64_t ipc_guid_t;

/*!
 * IPC representation of one's own snake and current world parameters.
 */
struct ALIGNED IpcSelfInfo {
	ipc_real_t segment_radius; //!< Radius of your snake's segments
	ipc_real_t mass;           //!< Your Snake's current mass
	ipc_real_t sight_radius;   //!< Radius around your snake's head in which you can see food and segments
	ipc_real_t consume_radius; //!< Radius around your snake's head in which food is consumed.

	uint32_t start_frame;   //!< Frame number when your snake was spawned
	uint32_t current_frame; //!< Current frame number

	ipc_real_t speed;          //!< Distance per step
	ipc_real_t max_step_angle; //!< Maximum direction change in this step

	ipc_real_t consumed_natural_food;          //!< Amount of "naturally" spawned food your snake consumed
	ipc_real_t consumed_food_hunted_by_self;   //!< Amount of food you consumed and that was created from snakes you killed
	ipc_real_t consumed_food_hunted_by_others; //!< Amount of food you consumed and that was created from snakes others killed (carrion)
};

/*!
 * IPC representation of the static server and world configuration.
 *
 * Please note that this struct is filled once before bot startup, so if you
 * (accidentally) modify them, they stay that way until your bot restarts.
 */
struct ALIGNED IpcServerConfig {
	uint32_t   snake_boost_steps;        //!< Number of steps a snake moves per frame while boosting
	ipc_real_t snake_turn_radius_factor; //!< Multiplied with your segment radius to determine the inner turn radius

	ipc_real_t snake_pull_factor; //!< Pull-together factor (determines how fast segments move to the center of a loop)

	ipc_real_t snake_conversion_factor; //!< how much of a snake's mass is converted to food when it dies

	ipc_real_t snake_segment_distance_factor;   //!< segment distance = (mass * factor)^exponent
	ipc_real_t snake_segment_distance_exponent; //!< segment distance = (mass * factor)^exponent

	ipc_real_t snake_consume_range; //!< consume range multiplier (multiplied with segment radius)

	ipc_real_t snake_boost_loss_factor;    //!< Multiplied with the snakes mass to determine how much mass is lost per frame while boosting
	ipc_real_t snake_survival_loss_factor; //!< This part of your mass is dropped every frame (to limit snake growth)

	ipc_real_t snake_self_kill_mass_threshold; //!< Mass below which a snake dies through starvation

	ipc_real_t food_decay_step; //!< Food decays by this value each frame

	ipc_real_t log_credits_per_frame; //!< How many log messages you can send per frame
	ipc_real_t log_initial_credits;   //!< How many log messages you can send right after startup
	ipc_real_t log_max_credits;       //!< You can send at most this many messages in a row
};

/*!
 * IPC representation of a food particle.
 */
struct ALIGNED IpcFoodInfo {
	ipc_real_t x;      //!< Relative position X in world orientation
	ipc_real_t y;      //!< Relative position Y in world orientation
	ipc_real_t val;    //!< Food value
	ipc_real_t dir;    //!< Direction angle relative to your heading (range -π to +π)
	ipc_real_t dist;   //!< Distance measured from the center of your head
};

/*!
 * IPC representation of a bot.
 */
struct ALIGNED IpcBotInfo {
	ipc_guid_t bot_id;        //!< Bot ID
	char       bot_name[64];  //!< Bot name (the beginning of it, at least)
};

/*!
 * IPC representation of a snake segment.
 */
struct ALIGNED IpcSegmentInfo {
	ipc_real_t x;       //!< Relative position X in world orientation
	ipc_real_t y;       //!< Relative position Y in world orientation
	ipc_real_t r;       //!< Segment radius
	ipc_real_t dir;     //!< Direction angle relative to your heading (range -π to +π)
	ipc_real_t dist;    //!< Distance between the center of your head and the segment's center
	uint32_t   idx;     //!< Segment number starting from head (idx == 0)
	ipc_guid_t bot_id;  //!< Bot ID
	bool       is_self; //!< True if this segment belongs to ones own snake
};

/*!
 * IPC representation of a color value.
 */
struct ALIGNED IpcColor {
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

const size_t IPC_PERSISTENT_MAX_BYTES = 4096; //!< Space for persistent data (in bytes)

/*!
 * Shared memory structure.
 *
 * This structure represents the contents of the memory the bot shares with the
 * gameserver.
 */
struct IpcSharedMemory {
	struct IpcServerConfig serverConfig; //!< Information about the world and server configuration.

	struct IpcSelfInfo selfInfo; //!< Information about your snake (updated every frame).

	uint32_t foodCount;                              //!< Number of items used in foodInfo.
	struct IpcFoodInfo foodInfo[IPC_FOOD_MAX_COUNT]; //!< List of food items seen by the snake.

	uint32_t botCount;                            //!< Number of items used in botInfo.
	struct IpcBotInfo botInfo[IPC_BOT_MAX_COUNT]; //!< List of bots related to segments in segmentInfo.

	uint32_t segmentCount;                                    //!< Number of items used in segmentInfo.
	struct IpcSegmentInfo segmentInfo[IPC_SEGMENT_MAX_COUNT]; //!< List of segments seen by the snake.

	uint32_t colorCount;                         //!< Number of items used in colors.
	struct IpcColor colors[IPC_COLOR_MAX_COUNT]; //!< Colors to set for this snake.

	char logData[IPC_LOG_MAX_BYTES]; //!< Log data for the current frame. May contain multiple lines.

	uint32_t faceID;   //!< Select a face for your snake (not used yet).
	uint32_t dogTagID; //!< Select a dog tag for your snake (not used yet).

	uint8_t persistentData[IPC_PERSISTENT_MAX_BYTES]; //!< Persistent data: will be saved after your snake dies and restored when it respawns
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
struct ALIGNED IpcRequest {
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
struct ALIGNED IpcResponse {
	enum IpcResponseType type;

	union ALIGNED {
		struct ALIGNED {
			ipc_real_t deltaAngle; //!< Direction change in this frame (radians, -π to +π).
			bool       boost;      //!< Set to true to boost.
		} step;
	};
};

} // extern "C"
