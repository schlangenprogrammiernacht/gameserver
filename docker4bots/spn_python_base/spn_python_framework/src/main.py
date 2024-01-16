#!/usr/bin/env python3

"""Main module providing the framework calling the bot code provided by the user"""

import sys
import traceback
import os
import socket
import struct

sys.path.insert(0, '/spndata')
from usercode import init,step
from api import Api

class SpnBot():
	"""Main class handling communication with the gameserver and calling bot code provided by the user"""
	RX_REQ_INIT=0
	RX_REQ_STEP=1
	__socket = None
	__shm_fd = None

	def __init__(self):
		self.__socket = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
		self.__socket.connect("/spnshm/socket")
		self.__shm_fd = os.open("/spnshm/shm", os.O_RDWR)
		self.__api = Api(self.__shm_fd)

	def __del__(self):
		if self.__socket:
			self.__socket.close()
		if self.__shm_fd:
			os.close(self.__shm_fd)

	def run(self):
		"""main event loop"""
		angle = 0.0
		boost = False
		success = False

		while True:
			# wait for command from server
			in_data = self.__socket.recv(4, 0)

			if len(in_data) != 0:
				request = struct.unpack("I", in_data)[0]
			else:
				break # shutdown request from server

			try:
				if self.RX_REQ_STEP == request:
					success, angle, boost = step(self.__api) # call usercode
				elif self.RX_REQ_INIT == request:
					success = init(self.__api) # call usercode
				else:
					pass # unknown command
			except Exception:
				self.__api.log(traceback.format_exc(limit=-4)) # only print line the exception occured on
				success = False # kill bot

			out_data = struct.pack("Ifbxxx", int(not success), angle, boost)
			self.__socket.send(out_data)
		self.__socket.close()

main = SpnBot()
main.run()
