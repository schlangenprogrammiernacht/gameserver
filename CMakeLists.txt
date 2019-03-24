cmake_minimum_required (VERSION 3.2)
project (GameServer VERSION 0.1 LANGUAGES CXX)

add_subdirectory(lib/TcpServer/TcpServer)
add_subdirectory(dbg/print_shm)
#add_subdirectory(test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -pedantic")

find_package(Threads REQUIRED)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

include_directories(src)

# put all .cpp and .h files into the sources variable
set(sources
	src/Bot.cpp
	src/Bot.h
	src/BotThreadPool.cpp
	src/BotThreadPool.h
	src/BotUpDownThread.cpp
	src/BotUpDownThread.h
	src/config.h
	src/debug_funcs.h
	src/DockerBot.cpp
	src/DockerBot.h
	src/Field.cpp
	src/Field.h
	src/Food.cpp
	src/Food.h
	src/Game.cpp
	src/Game.h
	src/GUIDGenerator.cpp
	src/GUIDGenerator.h
	src/IdentifyableObject.cpp
	src/IdentifyableObject.h
	src/PositionObject.h
	src/main.cpp
	src/MsgPackProtocol.h
	src/MsgPackUpdateTracker.cpp
	src/MsgPackUpdateTracker.h
	src/Semaphore.h
	src/Snake.cpp
	src/Snake.h
	src/SpatialMap.h
	src/types.h
	src/UpdateTracker.h
	src/Environment.h
	src/Database.h src/Database.cpp
	src/Stopwatch.h src/Stopwatch.cpp
)

include_directories(
	${CMAKE_PROJECT_NAME}
	lib/msgpack-c/include/
	lib/TcpServer/TcpServer/include/
	docker4bots/spn_cpp_base/spn_cpp_framework/src/

	/usr/include/jdbc/  # FIXME: autodetect
	)

add_executable(
	${CMAKE_PROJECT_NAME}
	${sources}
	)

target_link_libraries(
	${CMAKE_PROJECT_NAME}
	tcpserver
	Threads::Threads
	mysqlcppconn
)
