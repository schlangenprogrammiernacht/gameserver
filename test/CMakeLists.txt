set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -pedantic")

include_directories(
	test_poolalloc
	../src/
	)

add_executable(
	test_poolalloc
	test_poolalloc.cpp
	../src/PoolAllocator.cpp
	)
