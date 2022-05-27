#!/usr/bin/env python3

import socket
import sys
import os
import struct
import mmap
import time
import math
import struct

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
	def __init__(self, shm):
		self.shm_fd = shm
		self.serverConfig = IpcServerConfig(self.shm_fd,           0)
		self.selfInfo     = IpcSelfInfo(self.shm_fd,              56)
		self.food         = IpcFoodInfo(self.shm_fd,             100)
		self.bots         = IpcBotInfo(self.shm_fd,          1048664)
		self.segments     = IpcSegmentInfo(self.shm_fd,      1122400)
		self.colors       = IpcColor(self.shm_fd,            2170968)
		self.persMemory   = IpcPersistentMemory(self.shm_fd, 2176100)
		self.faceId       = 0
		self.dogTagId     = 0

	def getServerConfig(self):
		return self.serverConfig

	def getSelfInfo(self):
		return self.selfInfo

	def getFood(self):
		return self.food

	def getSegments(self):
		return self.segments

	def getBots(self):
		return self.bots

	def clearColors(self):
		self.colors.clearColors()

	def addColor(self, r, g, b):
		self.colors.addColor(r, g, b)

	def readPersistentMemory(self, offset, length):
		return self.persMemory.read(offset, length)

	def writePersistentMemory(self, offset, data):
		self.persMemory.write(offset, data)

	def log(self, message):
		mem = mmap.mmap(self.shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_WRITE)
		if len(message) >= 1024:
			return
		mem.seek(2175068)
		mem.write(bytearray(message.encode("utf-8")))
		mem.write(bytes([0]))

	def setFaceId(self, id):
		self.faceId = id

	def setDogTagId(self, id):
		self.dogTagId = id



class IpcServerConfig():
	def __init__(self, shm_fd, byteOffset):
		self.mem = mmap.mmap(shm_fd, mmap.PAGESIZE, mmap.MAP_SHARED, mmap.PROT_READ)
		self.offset = byteOffset

	def getSnakeBoostSteps(self):
		"""Number of steps a snake moves per frame while boosting"""
		self.mem.seek(self.offset + 0*4)
		return struct.unpack("I", self.mem.read(4))[0]

	def getSnakeTurnRadiusFactor(self):
		"""Multiplied with your segment radius to determine the inner turn radius"""
		self.mem.seek(self.offset + 1*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getSnakePullFactor(self):
		"""Pull-together factor (determines how fast segments move to the center of a loop)"""
		self.mem.seek(self.offset + 2*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getSnakeConversionFactor(self):
		"""how much of a snake's mass is converted to food when it dies"""
		self.mem.seek(self.offset + 3*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getSnakeSegmentDistanceFactor(self):
		"""segment distance = (mass * factor)^exponent"""
		self.mem.seek(self.offset + 4*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getSnakeSegmentDistanceExponent(self):
		"""segment distance = (mass * factor)^exponent"""
		self.mem.seek(self.offset + 5*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getSnakeConsumeRange(self):
		"""consume range multiplier (multiplied with segment radius)"""
		self.mem.seek(self.offset + 6*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getSnakeBoostLossFactor(self):
		"""Multiplied with the snakes mass to determine how much mass is lost per frame while boosting"""
		self.mem.seek(self.offset + 7*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getSnakeSurvivalLossFactor(self):
		"""This part of your mass is dropped every frame (to limit snake growth)"""
		self.mem.seek(self.offset + 8*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getSnakeSelfKillMassThreshold(self):
		"""Mass below which a snake dies through starvation"""
		self.mem.seek(self.offset + 9*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getFoodDecayStep(self):
		"""Food decays by this value each frame"""
		self.mem.seek(self.offset + 10*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getLogCreditsPerFrame(self):
		"""How many log messages you can send per frame"""
		self.mem.seek(self.offset + 11*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getLogInitialCredits(self):
		"""How many log messages you can send right after startup"""
		self.mem.seek(self.offset + 12*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getLogMaxCredits(self):
		"""You can send at most this many messages in a row"""
		self.mem.seek(self.offset + 13*4)
		return struct.unpack("f", self.mem.read(4))[0]



class IpcSelfInfo():
	def __init__(self, shm_fd, byteOffset):
		self.mem = mmap.mmap(shm_fd, mmap.PAGESIZE, mmap.MAP_SHARED, mmap.PROT_READ)
		self.offset = byteOffset

	def getSegmentRadius(self):
		"""Radius of your snake's segments"""
		self.mem.seek(self.offset + 0*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getMass(self):
		"""Your Snake's current mass"""
		self.mem.seek(self.offset + 1*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getSightRadius(self):
		"""Radius around your snake's head in which you can see food and segments"""
		self.mem.seek(self.offset + 2*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getConsumeRadius(self):
		"""Radius around your snake's head in which food is consumed"""
		self.mem.seek(self.offset + 3*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getStartFrame(self):
		"""Frame number when your snake was spawned"""
		self.mem.seek(self.offset + 4*4)
		return struct.unpack("I", self.mem.read(4))[0]

	def getCurrentFrame(self):
		"""Current frame number"""
		self.mem.seek(self.offset + 5*4)
		return struct.unpack("I", self.mem.read(4))[0]

	def getSpeed(self):
		"""Distance per step"""
		self.mem.seek(self.offset + 6*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getMaxStepAngle(self):
		"""Maximum direction change in this step"""
		self.mem.seek(self.offset + 7*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getConsumedNaturalFood(self):
		"""Amount of "naturally" spawned food your snake consumed"""
		self.mem.seek(self.offset + 8*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getConsumedFoodHuntedBySelf(self):
		"""Amount of food you consumed and that was created from snakes you killed"""
		self.mem.seek(self.offset + 9*4)
		return struct.unpack("f", self.mem.read(4))[0]

	def getConsumedFoodHuntedByOthers(self):
		"""Amount of food you consumed and that was created from snakes others killed (carrion)"""
		self.mem.seek(self.offset + 10*4)
		return struct.unpack("f", self.mem.read(4))[0]



class IpcFoodInfo():
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
	elemCount = 0
	## current element served by iterator
	dataIdx = 0

	def __init__(self, shm_fd, byteOffset):
		self.mem = mmap.mmap(shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_READ)
		self.offset = byteOffset
		self.mem.seek(self.offset)
		self.elemCount = struct.unpack("I", self.mem.read(4))[0]

	def __len__(self):
		"""Get the length of the Food list"""
		return self.elemCount

	def __iter__(self):
		self.mem.seek(self.offset)
		self.elemCount = struct.unpack("I", self.mem.read(4))[0]
		self.dataIdx = 0
		return self

	def __next__(self):
		"""Get next Food list item"""
		if self.dataIdx >= self.elemCount:
			raise StopIteration
		self.mem.seek(self.offset + 4 + (4*5 * self.dataIdx))
		self.x, self.y, self.val, self.dir, self.dist = struct.unpack("fffff", self.mem.read(5*4))
		self.dataIdx += 1
		return self



class IpcBotInfo():
	## Bot ID
	botId = 0
	## Bot name (the beginning of it, at least)
	botName = ""

	def __init__(self, shm_fd, byteOffset):
		self.mem = mmap.mmap(shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_READ)
		self.offset = byteOffset
		self.mem.seek(self.offset)
		self.elemCount = struct.unpack("I", self.mem.read(4))[0]

	def __len__(self):
		"""Get the length of the Bot list"""
		return self.elemCount

	def __iter__(self):
		self.mem.seek(self.offset)
		self.elemCount = struct.unpack("I", self.mem.read(4))[0]
		self.dataIdx = 0
		return self

	def __next__(self):
		"""Get next Bot list item"""
		if self.dataIdx >= self.elemCount:
			raise StopIteration
		self.mem.seek(self.offset + 8 + ((8+64)*self.dataIdx))
		self.botId   = struct.unpack("Q", self.mem.read(8))[0]
		self.botName = str(self.mem.read(64).rstrip(b"\0").decode())
		self.dataIdx += 1
		return self



class IpcSegmentInfo():
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
	botId = 0
	## True if this segment belongs to ones own snake
	isSelf = False

	def __init__(self, shm_fd, byteOffset):
		self.mem = mmap.mmap(shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_READ)
		self.offset = byteOffset
		self.mem.seek(self.offset)
		self.elemCount = struct.unpack("I", self.mem.read(4))[0]

	def __len__(self):
		return self.elemCount

	def __iter__(self):
		self.mem.seek(self.offset)
		self.elemCount = struct.unpack("I", self.mem.read(4))[0]
		self.dataIdx = 0
		return self

	def __next__(self):
		if self.dataIdx > self.elemCount:
			raise StopIteration
		self.mem.seek(self.offset + 8 + (40 * self.dataIdx))
		self.x, self.y, self.r, self.dir, self.dist, self.idx, self.botId, self.isSelf = struct.unpack("fffffIQ?xxx", self.mem.read(36))
		self.dataIdx += 1
		return self



class IpcColor():
	## Red channel (0-255)
	r = 0
	## Green channel (0-255)
	g = 0
	## Blue channel (0-255)
	b = 0

	def __init__(self, shm_fd, byteOffset):
		self.mem = mmap.mmap(shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)
		self.offset = byteOffset
		self.mem.seek(self.offset)
		self.elemCount = 0

	def __len__(self):
		return self.elemCount

	def __iter__(self):
		self.mem.seek(self.offset)
		self.elemCount
		self.dataIdx = 0
		return self

	def __next__(self):
		if self.dataIdx > self.elemCount:
			raise StopIteration
		self.mem.seek(self.offset + 4 + (4 * self.dataIdx))
		self.r, self.g, self.b = struct.unpack("bbbx", self.mem.read(4))
		self.dataIdx += 1
		return self

	def addColor(self, r, g ,b):
		if self.elemCount >= 1024:
			return

		self.mem.seek(self.offset + 4 + (self.elemCount*4))
		self.mem.write(struct.pack("BBBx", int(r), int(g), int(b)))
		self.elemCount += 1
		self.mem.seek(self.offset)
		self.mem.write(struct.pack("I", self.elemCount))

	def clearColors(self):
		self.elemCount = 0
		self.mem.seek(self.offset)
		self.mem.write(struct.pack("I", self.elemCount))



class IpcPersistentMemory():
	def __init__(self, shm_fd, byteOffset):
		self.mem = mmap.mmap(shm_fd, 0, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)
		self.offset = byteOffset

	def read(self, offset, length):
		if (offset + length) > 4096:
			raise IndexError("reading past the buffer")
		self.mem.seek(self.offset + offset)
		return self.mem.read(length)

	def write(self, offset, data):
		if (offset + len(data)) > 4096:
			raise IndexError("writing past the buffer")
		self.mem.seek(self.offset + offset)
		self.mem.write(data)
