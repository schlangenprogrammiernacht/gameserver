#!/usr/bin/env python3

import sys
import os
import socket
import struct

sys.path.insert(0, '/spndata')
from usercode import init,step
from api import Api

class SpnBot():
	RX_REQ_INIT=0
	RX_REQ_STEP=1
	socket = None
	shm_fd = None

	def __init__(self):
		self.socket = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
		self.socket.connect("/spnshm/socket")
		self.shm_fd = os.open("/spnshm/shm", os.O_RDWR)
		self.api = Api(self.shm_fd)

	def __del__(self):
		if self.socket:
			self.socket.close()
		if self.shm_fd:
			os.close(self.shm_fd)

	def run(self):
		angle = 0.0
		boost = False
		success = False

		while True:
			# wait for command from server
			inData = self.socket.recv(4, 0)

			if len(inData) != 0:
				request = struct.unpack("I", inData)[0]
			else:
				break # shutdown request from server

			if self.RX_REQ_STEP == request:
				success, angle, boost = step(self.api) # call usercode
			elif self.RX_REQ_INIT == request:
				success = init(self.api) # call usercode
			else:
				pass # unknown command

			outData = struct.pack("Ifbxxx", int(not success), angle, boost)
			self.socket.send(outData)
		self.socket.close()

main = SpnBot()
main.run()
