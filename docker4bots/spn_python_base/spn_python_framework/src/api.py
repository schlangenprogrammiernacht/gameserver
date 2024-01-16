#!/usr/bin/env python3

"""The python API for bots written for the SchlangenProgrammierNacht"""

import struct
import mmap

# |       IpcSharedMemory offsets (shm)        |
# |      name       |  size   |  start offset  |
# |-----------------|---------|----------------|
# | IpcServerConfig |      56 |              0 |
# | IpcSelfInfo     |      44 |             56 |
# | foodCount       |       4 |            100 |
# | IpcFoodInfo     | 1048560 |            104 |
# | botCount        |       4 |        1048664 |
# | IpcBotInfo      |   73728 |        1048672 |
# | segmentCount    |       4 |        1122400 |
# | IpcSegmentInfo  | 1048560 |        1122408 |
# | colorCount      |       4 |        2170968 |
# | IpcColor        |    4096 |        2170972 |
# | logData         |    1024 |        2175068 |
# | faceId          |       4 |        2176092 |
# | dogTagId        |       4 |        2176096 |
# | persistentData  |    4096 |        2176100 |


class Api():
	"""
	Top level data class exposing configuration and some actions to bots
	"""

	face_id   = 0
	dog_tag_id = 0

	def __init__(self, shm):
		self.__shm_fd = shm
		self.server_config = IpcServerConfig(self.__shm_fd,           0)
		self.self_info     = IpcSelfInfo(self.__shm_fd,              56)
		self.food          = IpcFoodInfo(self.__shm_fd,             100)
		self.bots          = IpcBotInfo(self.__shm_fd,          1048664)
		self.segments      = IpcSegmentInfo(self.__shm_fd,      1122400)
		self.__colors      = IpcColor(self.__shm_fd,            2170968)
		self.__pers_memory = IpcPersistentMemory(self.__shm_fd, 2176100)

	def clear_colors(self):
		"""reset the color pattern of the bot"""
		self.__colors.clear_colors()

	def add_color(self, red, green, blue):
		"""add a color to the color sequence marking the bot"""
		self.__colors.add_color(red, green, blue)

	def read_persistent_memory(self, offset, length):
		"""read from only memory section that is persistent even over bot restarts"""
		return self.__pers_memory.read(offset, length)

	def write_persistent_memory(self, offset, data):
		"""write to only memory section that is persistent even over bot restarts"""
		self.__pers_memory.write(offset, data)

	def log(self, message):
		"""write a log message to the console"""
		message_str = str(message)
		mem = mmap.mmap(self.__shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_WRITE)
		if len(message_str) < 1024:
			mem.seek(2175068)
			mem.write(bytearray(message_str.encode("utf-8")))
			mem.write(bytes([0]))



class IpcServerConfig():
	"""
	Data class exposing the game server configuration to bots
	"""

	def __init__(self, shm_fd, byte_offset):
		self.__mem = mmap.mmap(shm_fd, mmap.PAGESIZE, mmap.MAP_SHARED, mmap.PROT_READ)
		self.__offset = byte_offset

	@property
	def snake_boost_steps(self):
		"""Number of steps a snake moves per frame while boosting"""
		self.__mem.seek(self.__offset + 0*4)
		return struct.unpack("I", self.__mem.read(4))[0]

	@property
	def snake_turn_radius_factor(self):
		"""Multiplied with your segment radius to determine the inner turn radius"""
		self.__mem.seek(self.__offset + 1*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def snake_pull_factor(self):
		"""Pull-together factor (determines how fast segments move to the center of a loop)"""
		self.__mem.seek(self.__offset + 2*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def snake_conversion_factor(self):
		"""how much of a snake's mass is converted to food when it dies"""
		self.__mem.seek(self.__offset + 3*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def snake_segment_distance_factor(self):
		"""segment distance = (mass * factor)^exponent"""
		self.__mem.seek(self.__offset + 4*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def snake_segment_distance_exponent(self):
		"""segment distance = (mass * factor)^exponent"""
		self.__mem.seek(self.__offset + 5*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def snake_consume_range(self):
		"""consume range multiplier (multiplied with segment radius)"""
		self.__mem.seek(self.__offset + 6*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def snake_boost_loss_factor(self):
		"""Multiplied with the snakes mass to determine how much mass is lost per frame while boosting"""
		self.__mem.seek(self.__offset + 7*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def snake_survival_loss_factor(self):
		"""This part of your mass is dropped every frame (to limit snake growth)"""
		self.__mem.seek(self.__offset + 8*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def snake_self_kill_mass_threshold(self):
		"""Mass below which a snake dies through starvation"""
		self.__mem.seek(self.__offset + 9*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def food_decay_step(self):
		"""Food decays by this value each frame"""
		self.__mem.seek(self.__offset + 10*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def log_credits_per_frame(self):
		"""How many log messages you can send per frame"""
		self.__mem.seek(self.__offset + 11*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def log_initial_credits(self):
		"""How many log messages you can send right after startup"""
		self.__mem.seek(self.__offset + 12*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def log_max_credits(self):
		"""You can send at most this many messages in a row"""
		self.__mem.seek(self.__offset + 13*4)
		return struct.unpack("f", self.__mem.read(4))[0]





class IpcSelfInfo():
	"""
	Data class exposing general information about this bot
	"""

	def __init__(self, shm_fd, byte_offset):
		self.__mem = mmap.mmap(shm_fd, mmap.PAGESIZE, mmap.MAP_SHARED, mmap.PROT_READ)
		self.__offset = byte_offset

	@property
	def segment_radius(self):
		"""Radius of your snake's segments"""
		self.__mem.seek(self.__offset + 0*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def mass(self):
		"""Your Snake's current mass"""
		self.__mem.seek(self.__offset + 1*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def sight_radius(self):
		"""Radius around your snake's head in which you can see food and segments"""
		self.__mem.seek(self.__offset + 2*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def consume_radius(self):
		"""Radius around your snake's head in which food is consumed"""
		self.__mem.seek(self.__offset + 3*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def start_frame(self):
		"""Frame number when your snake was spawned"""
		self.__mem.seek(self.__offset + 4*4)
		return struct.unpack("I", self.__mem.read(4))[0]

	@property
	def current_frame(self):
		"""Current frame number"""
		self.__mem.seek(self.__offset + 5*4)
		return struct.unpack("I", self.__mem.read(4))[0]

	@property
	def speed(self):
		"""Distance per step"""
		self.__mem.seek(self.__offset + 6*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def max_step_angle(self):
		"""Maximum direction change in this step"""
		self.__mem.seek(self.__offset + 7*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def consumed_natural_food(self):
		"""Amount of "naturally" spawned food your snake consumed"""
		self.__mem.seek(self.__offset + 8*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def consumed_food_hunted_by_self(self):
		"""Amount of food you consumed and that was created from snakes you killed"""
		self.__mem.seek(self.__offset + 9*4)
		return struct.unpack("f", self.__mem.read(4))[0]

	@property
	def consumed_food_hunted_by_others(self):
		"""Amount of food you consumed and that was created from snakes others killed (carrion)"""
		self.__mem.seek(self.__offset + 10*4)
		return struct.unpack("f", self.__mem.read(4))[0]



class IpcFoodInfo():
	"""
	Data class exposing information about size and location of food objects
	"""

	## Relative position X in world orientation
	x    = 0.0
	## Relative position Y in world orientation
	y    = 0.0
	## Food value
	val  = 0.0
	## Direction angle relative to your heading (range -π to +π)
	dir  = 0.0
	## Distance measured from the center of your head
	dist = 0.0
	## number of food elements to iterate over
	elem_count = 0
	## current element served by iterator
	__data_idx = 0

	def __init__(self, shm_fd, byte_offset):
		self.__mem = mmap.mmap(shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_READ)
		self.__offset = byte_offset
		self.__mem.seek(self.__offset)
		self.__elem_count = struct.unpack("I", self.__mem.read(4))[0]

	def __len__(self):
		"""Get the length of the Food list"""
		return self.__elem_count

	def __iter__(self):
		self.__mem.seek(self.__offset)
		self.__elem_count = struct.unpack("I", self.__mem.read(4))[0]
		self.__data_idx = 0
		return self

	def __next__(self):
		"""Get next Food list item"""
		if self.__data_idx >= self.__elem_count:
			raise StopIteration
		self.__mem.seek(self.__offset + 4 + (4*5 * self.__data_idx))
		self.x, self.y, self.val, self.dir, self.dist = struct.unpack("fffff", self.__mem.read(5*4))
		self.__data_idx += 1
		return self

	def __getitem__(self, idx):
		"""get element at index but don't affect iteration"""
		if idx >= self.__elem_count:
			raise IndexError
		self.__mem.seek(self.__offset + 4 + (4*5 * idx))
		self.x, self.y, self.val, self.dir, self.dist = struct.unpack("fffff", self.__mem.read(5*4))
		return self


class IpcBotInfo():
	"""
	Data class exposing a mapping of bot_ids to bot names in the bots range of vision
	"""

	## Bot ID
	bot_id = 0
	## Bot name (the beginning of it, at least)
	bot_name = ""
	## current element served by iterator
	__data_idx = 0

	def __init__(self, shm_fd, byte_offset):
		self.__mem = mmap.mmap(shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_READ)
		self.__offset = byte_offset
		self.__mem.seek(self.__offset)
		self.__elem_count = struct.unpack("I", self.__mem.read(4))[0]

	def __len__(self):
		"""Get the length of the Bot list"""
		return self.__elem_count

	def __iter__(self):
		self.__mem.seek(self.__offset)
		self.__elem_count = struct.unpack("I", self.__mem.read(4))[0]
		self.__data_idx = 0
		return self

	def __next__(self):
		"""Get next Bot list item"""
		if self.__data_idx >= self.__elem_count:
			raise StopIteration
		self.__mem.seek(self.__offset + 8 + ((8+64) * self.__data_idx))
		self.bot_id   = struct.unpack("Q", self.__mem.read(8))[0]
		self.bot_name = str(self.__mem.read(64).rstrip(b"\0").decode())
		self.__data_idx += 1
		return self

	def __getitem__(self, idx):
		"""get element at index but don't affect iteration"""
		if idx >= self.__elem_count:
			raise IndexError
		self.__mem.seek(self.__offset + 8 + ((8+64) * idx))
		self.bot_id   = struct.unpack("Q", self.__mem.read(8))[0]
		self.bot_name = str(self.__mem.read(64).rstrip(b"\0").decode())
		return self


class IpcSegmentInfo():
	"""
	Data class exposing information about size and location of other bots in the range of vision of this bot
	"""

	## Relative position X in world orientation
	x = 0.0
	## Relative position Y in world orientation
	y = 0.0
	## Segment radius
	r = 0.0
	## Direction angle relative to your heading (range -π to +π)
	dir = 0.0
	## Distance between the center of your head and the segment's center
	dist = 0.0
	## Segment number starting from head (idx == 0)
	idx = 0
	## Bot ID
	bot_id = 0
	## True if this segment belongs to ones own snake
	is_self = False
	## current element served by iterator
	__data_idx = 0

	def __init__(self, shm_fd, byte_offset):
		self.__mem = mmap.mmap(shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_READ)
		self.__offset = byte_offset
		self.__mem.seek(self.__offset)
		self.__elem_count = struct.unpack("I", self.__mem.read(4))[0]

	def __len__(self):
		"""Get the length of the Segment list"""
		return self.__elem_count

	def __iter__(self):
		"""Get next Bot list item"""
		self.__mem.seek(self.__offset)
		self.__elem_count = struct.unpack("I", self.__mem.read(4))[0]
		self.__data_idx = 0
		return self

	def __next__(self):
		"""Get next Bot list item"""
		if self.__data_idx >= self.__elem_count:
			raise StopIteration
		self.__mem.seek(self.__offset + 8 + (40 * self.__data_idx))
		self.x, self.y, self.r, self.dir, self.dist, self.idx, self.bot_id, self.is_self = struct.unpack("fffffIQ?xxx", self.__mem.read(36))
		self.__data_idx += 1
		return self

	def __getitem__(self, idx):
		"""get element at index but don't affect iteration"""
		if idx >= self.__elem_count:
			raise IndexError(f"idx({idx}) >= elem_count({self.__elem_count})")
		self.__mem.seek(self.__offset + 8 + (40 * self.idx))
		self.x, self.y, self.r, self.dir, self.dist, self.idx, self.bot_id, self.is_self = struct.unpack("fffffIQ?xxx", self.__mem.read(36))
		return self


class IpcColor():
	"""
	Data class exposing information about the color pattern of this bot
	"""

	## Red channel (0-255)
	r = 0
	## Green channel (0-255)
	g = 0
	## Blue channel (0-255)
	b = 0
	## current element served by iterator
	__data_idx = 0

	def __init__(self, shm_fd, byte_offset):
		self.__mem = mmap.mmap(shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)
		self.__offset = byte_offset
		self.__mem.seek(self.__offset)
		self.__elem_count = struct.unpack("I", self.__mem.read(4))[0]


	def __len__(self):
		"""Get the length of the Color list"""
		return self.__elem_count

	def __iter__(self):
		self.__mem.seek(self.__offset)
		self.__elem_count = struct.unpack("I", self.__mem.read(4))[0]
		self.__data_idx = 0
		return self

	def __next__(self):
		"""Get next Color list item"""
		if self.__data_idx > self.__elem_count:
			raise StopIteration
		self.__mem.seek(self.__offset + 4 + (4 * self.__data_idx))
		self.r, self.g, self.b = struct.unpack("BBBx", self.__mem.read(4))
		self.__data_idx += 1
		return self

	def __getitem__(self, idx):
		"""get element at index but don't affect iteration"""
		if idx >= self.__elem_count:
			raise IndexError(f"idx({idx}) >= elem_count({self.__elem_count})")

		self.__mem.seek(self.__offset + 4 + (4 * idx))
		self.r, self.g, self.b = struct.unpack("BBBx", self.__mem.read(4))
		return self

	def add_color(self, r, g ,b):
		"""add a color to the color sequence marking the bot"""
		if self.__elem_count >= 1024:
			return

		self.__mem.seek(self.__offset + 4 + (self.__elem_count * 4))
		self.__mem.write(struct.pack("BBBx", int(r), int(g), int(b)))
		self.__elem_count += 1
		self.__mem.seek(self.__offset)
		self.__mem.write(struct.pack("I", self.__elem_count))

	def clear_colors(self):
		"""reset the color pattern of the bot"""
		self.__elem_count = 0
		self.__mem.seek(self.__offset)
		self.__mem.write(struct.pack("I", self.__elem_count))



class IpcPersistentMemory():
	"""
	Provides an interface to persistant memory to this bot
	"""

	def __init__(self, shm_fd, byte_offset):
		self.__mem = mmap.mmap(shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)
		self.__offset = byte_offset

	def read(self, offset, length):
		"""read from only memory section that is persistent even over bot restarts"""
		if (offset + length) > 4096:
			raise IndexError(f"offset({offset}) + length({length}) > 4096. reading past the buffer")
		self.__mem.seek(self.__offset + offset)
		return self.__mem.read(length)

	def write(self, offset, data):
		"""write to only memory section that is persistent even over bot restarts"""
		if (offset + len(data)) > 4096:
			raise IndexError(f"offset({offset}) + length({len(data)}) > 4096. writing past the buffer")
		self.__mem.seek(self.__offset + offset)
		self.__mem.write(data)
