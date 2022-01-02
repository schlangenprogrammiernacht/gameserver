use std::mem::size_of;

type IpcReal = f32;
type IpcGuid = u64;

/// IPC representation of one's own snake and current world parameters.
#[repr(C)]
#[repr(align(4))]
pub struct IpcSelfInfo {
	/// Radius of your snake's segments
	pub segment_radius                 : IpcReal,
	/// Your Snake's current mass
	pub mass                           : IpcReal,
	/// Radius around your snake's head in which you can see food and segments
	pub sight_radius                   : IpcReal,
	/// Radius around your snake's head in which food is consumed.
	pub consume_radius                 : IpcReal,

	/// Frame number when your snake was spawned
	pub start_frame                    : u32,
	/// Current frame number
	pub current_frame                  : u32,

	/// Distance per step
	pub speed                          : IpcReal,
	/// Maximum direction change in this step
	pub max_step_angle                 : IpcReal,

	/// Amount of "naturally" spawned food your snake consumed
	pub consumed_natural_food          : IpcReal,
	/// Amount of food you consumed and that was created from snakes you killed
	pub consumed_food_hunted_by_self   : IpcReal,
	/// Amount of food you consumed and that was created from snakes others killed (carrion)
	pub consumed_food_hunted_by_others : IpcReal,
}

/**
 * IPC representation of the static server and world configuration.
 *
 * Please note that this struct is filled once before bot startup, so if you
 * (accidentally) modify them, they stay that way until your bot restarts.
 */
#[repr(C)]
#[repr(align(4))]
pub struct IpcServerConfig {
	/// Number of steps a snake moves per frame while boosting
	pub snake_boost_steps: u32,
	/// Multiplied with your segment radius to determine the inner turn radius
	pub snake_turn_radius_factor: IpcReal,

	/// Pull-together factor (determines how fast segments move to the center of a loop)
	pub snake_pull_factor: IpcReal,

	/// how much of a snake's mass is converted to food when it dies
	pub snake_conversion_factor: IpcReal,

	/// segment distance = (mass * factor)^exponent
	pub snake_segment_distance_factor: IpcReal,
	/// segment distance = (mass * factor)^exponent
	pub snake_segment_distance_exponent: IpcReal,

	/// consume range multiplier (multiplied with segment radius)
	pub snake_consume_range: IpcReal,

	/// Multiplied with the snakes mass to determine how much mass is lost per frame while boosting
	pub snake_boost_loss_factor: IpcReal,
	/// This part of your mass is dropped every frame (to limit snake growth)
	pub snake_survival_loss_factor: IpcReal,

	/// Mass below which a snake dies through starvation
	pub snake_self_kill_mass_theshold: IpcReal,

	/// Food decays by this value each frame
	pub food_decay_step: IpcReal,

	/// How many log messages you can send per frame
	pub log_credits_per_frame: IpcReal,
	/// How many log messages you can send right after startup
	pub log_initial_credits: IpcReal,
	/// You can send at most this many messages in a row
	pub log_max_credits: IpcReal,
}

/**
 * IPC representation of a food particle.
 */
#[repr(C)]
#[repr(align(4))]
pub struct IpcFoodInfo {
	/// Relative position X in world orientation
	pub x: IpcReal,
	/// Relative position Y in world orientation
	pub y: IpcReal,
	/// Food value
	pub val: IpcReal,
	/// Direction angle relative to your heading (range -π to +π)
	pub dir: IpcReal,
	/// Distance measured from the center of your head
	pub dist: IpcReal,
}

/**
 * IPC representation of a bot.
 */
#[repr(C)]
#[repr(align(4))]
pub struct IpcBotInfo {
	/// Bot ID
	pub bot_id: IpcGuid,
	/// Bot name (the beginning of it, at least)
	pub bot_name: [u8; 64],
}

/**
 * IPC representation of a snake segment.
 */
#[repr(C)]
#[repr(align(4))]
pub struct IpcSegmentInfo {
	/// Relative position X in world orientation
	pub x: IpcReal,
	/// Relative position Y in world orientation
	pub y: IpcReal,
	/// Segment radius
	pub r: IpcReal,
	/// Direction angle relative to your heading (range -π to +π)
	pub dir: IpcReal,
	/// Distance between the center of your head and the segment's center
	pub dist: IpcReal,
	/// Segment number starting from head (idx == 0)
	pub idx: u32,
	/// Bot ID
	pub bot_id: IpcGuid,
	/// True if this segment belongs to ones own snake
	pub is_self: bool,
}

/**
 * IPC representation of a color value.
 */
#[repr(C)]
#[repr(align(4))]
pub struct IpcColor {
	/// Red channel (0-255)
	pub r: u8,
	/// Green channel (0-255)
	pub g: u8,
	/// Blue channel (0-255)
	pub b: u8,
}

const IPC_FOOD_MAX_BYTES: usize = 1 * 1024*1024;
const IPC_FOOD_MAX_COUNT: usize = IPC_FOOD_MAX_BYTES / size_of::<IpcFoodInfo>();

const IPC_BOT_MAX_COUNT: usize = 1024;
const IPC_BOT_MAX_BYTES: usize = IPC_BOT_MAX_COUNT * size_of::<IpcBotInfo>();

const IPC_SEGMENT_MAX_BYTES: usize = 1 * 1024*1024;
const IPC_SEGMENT_MAX_COUNT: usize = IPC_SEGMENT_MAX_BYTES / size_of::<IpcSegmentInfo>();

const IPC_COLOR_MAX_COUNT: usize = 1024;
const IPC_COLOR_MAX_BYTES: usize = IPC_COLOR_MAX_COUNT * size_of::<IpcColor>();

const IPC_LOG_MAX_BYTES: usize = 1024;

/// Space for persistent data (in bytes)
const IPC_PERSISTENT_MAX_BYTES: usize = 4096;

/**
 * Shared memory structure.
 *
 * This structure represents the contents of the memory the bot shares with the
 * gameserver.
 */
#[repr(C)]
#[repr(align(4))]
pub struct IpcSharedMemory {
	/// Information about the world and server configuration.
	pub server_config: IpcServerConfig,

	/// Information about your snake (updated every frame).
	pub self_info: IpcSelfInfo,

	/// Number of items used in foodInfo.
	pub food_count: u32,
	/// List of food items seen by the snake.
	pub food_info: [IpcFoodInfo; IPC_FOOD_MAX_COUNT],

	/// Number of items used in botInfo.
	pub bot_count: u32,
	/// List of bots related to segments in segmentInfo.
	pub bot_info: [IpcBotInfo; IPC_BOT_MAX_COUNT],

	/// Number of items used in segmentInfo.
	pub segment_count: u32,
	/// List of segments seen by the snake.
	pub segment_info: [IpcSegmentInfo; IPC_SEGMENT_MAX_COUNT],

	/// Number of items used in colors.
	pub color_count: u32,
	/// Colors to set for this snake.
	pub colors: [IpcColor; IPC_COLOR_MAX_COUNT],

	/// Log data for the current frame. May contain multiple lines.
	pub log_data: [u8; IPC_LOG_MAX_BYTES],

	/// Select a face for your snake (not used yet).
	pub face_id: u32,
	/// Select a dog tag for your snake (not used yet).
	pub dog_tag_id: u32,

	/// Persistent data: will be saved after your snake dies and restored when it respawns
	pub persistent_data: [u8; IPC_PERSISTENT_MAX_BYTES],
}

const IPC_SHARED_MEMORY_BYTES: usize = size_of::<IpcSharedMemory>();

/*
 * Communication structures.
 */

#[repr(C)]
#[repr(align(4))]
pub enum IpcRequestType {
	/// Bot initialization request. Sent once after bot startup.
	Init = 0,
	/// Bot step request. Sent every frame.
	Step = 1
}

/**
 * A request to the bot by the gameserver.
 *
 * Sent by the gameserver on the control socket.
 */
#[repr(C)]
#[repr(align(4))]
pub struct IpcRequest {
	pub request_type: IpcRequestType
}

#[repr(C)]
#[repr(align(4))]
pub enum IpcResponseType {
	Ok = 0,
	Error = 1
}

#[repr(C)]
#[repr(align(4))]
#[derive(Clone, Copy)]
pub struct IpcStepResponse {
	/// Direction change in this frame (radians, -π to +π).
	pub delta_angle: IpcReal,
	/// Set to true to boost.
	pub boost: bool,
}

// FIXME! This should actually be a union, which makes things complicated in Rust. As there is
// currently only one entry anyway, we keep ResponseData as a struct and worry about the union
// later.
#[repr(C)]
#[repr(align(4))]
pub union ResponseData {
	pub step: IpcStepResponse
}

/**
 * Response of the Bot.
 *
 * Sent by the bot on the control socket.
 */
#[repr(C)]
#[repr(align(4))]
pub struct IpcResponse {
	pub response_type: IpcResponseType,

	pub data: ResponseData
}
