include_directories(
	test_poolalloc
	../src/
	)

add_executable(
	test_poolalloc
	test_poolalloc.cpp
	../src/PoolAllocator.cpp
	)

#target_link_libraries(
#	test_poolalloc
#)
