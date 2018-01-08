cmake_minimum_required (VERSION 2.6)
project (GameServer)

# put all .cpp and .h files into the sources variable
file(GLOB_RECURSE sources src/*.cpp src/*.h)

add_executable(GameServer ${sources})
