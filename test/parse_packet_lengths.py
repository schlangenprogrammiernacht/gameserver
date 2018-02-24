#!/usr/bin/env python3

import socket
import struct
import msgpack

s = socket.create_connection( ('localhost', 9010) )

f = s.makefile('b')

while f:
    lengthstr = f.read(4)

    length, = struct.unpack('>I', lengthstr)

    print("Read length: {:d}".format(length))

    data = b''
    while len(data) < length:
        data += f.read(length - len(data))

    print(msgpack.loads(data))
