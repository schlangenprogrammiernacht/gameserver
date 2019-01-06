cmake_minimum_required (VERSION 3.2)
project (bot VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -pedantic")

include_directories(src)

# put all .cpp and .h files into the sources variable
set(sources
	src/main.cpp
)

include_directories(
	${CMAKE_PROJECT_NAME}
	)

add_executable(
	${CMAKE_PROJECT_NAME}
	${sources}
	)

target_link_libraries(
	${CMAKE_PROJECT_NAME}
	rt
)
