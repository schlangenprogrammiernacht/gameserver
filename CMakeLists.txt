cmake_minimum_required (VERSION 3.2)
project (GameServer VERSION 0.1 LANGUAGES CXX)

add_subdirectory(lib/TcpServer/TcpServer)
add_subdirectory(test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -pedantic")

#set(LUA_INCLUDE_DIR "/usr/include/luajit-2.0")
#set(LUA_LIB "luajit-5.1")
set(LUA_INCLUDE_DIR "/usr/include/lua5.3")
set(LUA_LIB "lua5.3")

find_package(Threads REQUIRED)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# put all .cpp and .h files into the sources variable
set(sources
	src/Bot.cpp
	src/Bot.h
	src/BotThreadPool.cpp
	src/BotThreadPool.h
	src/LuaBot.cpp
	src/LuaBot.h
	src/config.h
	src/debug_funcs.h
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
	src/PoolAllocator.cpp
	src/PoolAllocator.h
	src/Semaphore.h
	src/Snake.cpp
	src/Snake.h
	src/SpatialMap.h
	src/types.h
	src/UpdateTracker.h

	src/Environment.h
	src/Database.h src/Database.cpp
)

include_directories(
	${CMAKE_PROJECT_NAME}
	lib/msgpack-c/include/
	lib/TcpServer/TcpServer/include/
	lib/sol2
	${LUA_INCLUDE_DIR}
	)

add_executable(
	${CMAKE_PROJECT_NAME}
	${sources}
	)

target_link_libraries(
	${CMAKE_PROJECT_NAME}
	tcpserver
	${LUA_LIB}
	Threads::Threads
	mysqlcppconn
)

configure_file("lua/demobot.lua" "lua/demobot.lua" COPYONLY)
configure_file("lua/quota.lua" "lua/quota.lua" COPYONLY)
