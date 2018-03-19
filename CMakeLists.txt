cmake_minimum_required (VERSION 3.2)
project (GameServer VERSION 0.1 LANGUAGES CXX)

add_subdirectory(lib/TcpServer/TcpServer)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -pedantic")

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

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
)

include_directories(
	${CMAKE_PROJECT_NAME}
	lib/msgpack-c/include/
	lib/TcpServer/TcpServer/include/
)

add_executable(
	${CMAKE_PROJECT_NAME}
	${sources}
)

target_link_libraries(
	${CMAKE_PROJECT_NAME}
	tcpserver
)
