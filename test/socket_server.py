#!/usr/bin/env python3

from socket import *
import os
import time

SOCKNAME = "/mnt/spn_shm/testbot/socket"

sock = socket(AF_UNIX, SOCK_SEQPACKET, 0)
sock.bind(SOCKNAME)

os.chmod(SOCKNAME, 0o666)

sock.listen(1)

print("Listening...")

clientsock, clientaddr = sock.accept()

print("Client connected!")

os.unlink(SOCKNAME)

for i in range(10):
    clientsock.send(f"Hello World x{i}".encode('utf-8'))
    time.sleep(1)

clientsock.close()

sock.close()
