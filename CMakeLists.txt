cmake_minimum_required (VERSION 2.6)
project (GameServer)

add_subdirectory(lib/TcpServer/TcpServer/)

# put all .cpp and .h files into the sources variable
set(sources
	src/Bot.cpp
	src/Bot.h
	src/config.h
	src/debug_funcs.h
	src/Field.cpp
	src/Field.h
	src/Food.cpp
	src/Food.h
	src/Game.cpp
	src/Game.h
	src/GlobalView.cpp
	src/GlobalView.h
	src/GUIDGenerator.cpp
	src/GUIDGenerator.h
	src/IdentifyableObject.cpp
	src/IdentifyableObject.h
	src/LocalView.cpp
	src/LocalView.h
	src/main.cpp
	src/MsgPackProtocol.h
	src/MsgPackUpdateTracker.cpp
	src/MsgPackUpdateTracker.h
	src/Snake.cpp
	src/Snake.h
	src/types.h
	src/UpdateTracker.h
	src/Vector.cpp
	src/Vector.h
	)

add_executable(GameServer ${sources})
target_include_directories(GameServer PUBLIC
	lib/TcpServer/TcpServer/include/
	lib/msgpack-c/include/)
target_link_libraries(GameServer tcpserver)
