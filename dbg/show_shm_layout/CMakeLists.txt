project (show_shm_layout VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -pedantic")

include_directories(src)

# put all .cpp and .h files into the sources variable
set(sources
	../../docker4bots/spn_cpp_base/spn_cpp_framework/src/ipc_format.h
	src/main.cpp
)

include_directories(
	${CMAKE_PROJECT_NAME}
	../../docker4bots/spn_cpp_base/spn_cpp_framework/src/
	)

add_executable(
	show_shm_layout
	${sources}
	)
