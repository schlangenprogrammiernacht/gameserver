#include <cassert>
#include <iostream>
#include <algorithm>

#include <cstdint>

#include "PoolAllocator.h"

void test_issue_6(PoolAllocator *alloc)
{
	uint8_t *growing = reinterpret_cast<uint8_t*>(alloc->allocate(100));
	uint8_t *growing2 = reinterpret_cast<uint8_t*>(alloc->reallocate(growing, 1024));

	assert(growing == growing2); // block should not be moved

	alloc->debugPrint();

	uint8_t *second = reinterpret_cast<uint8_t*>(alloc->allocate(100));

	alloc->debugPrint();
	assert(second >= growing+1024);

	alloc->deallocate(growing);
	alloc->deallocate(second);
}

int main(void)
{
	srand(1337); // for reproducible results

	PoolAllocator alloc(16384, 256);

	alloc.debugPrint();

	test_issue_6(&alloc);

	std::vector<void*> ptrs;

	for(int i = 0; i < 21; i++) {
		void *ptr = alloc.allocate(700);
		assert(ptr);
		ptrs.push_back(ptr);
		alloc.debugPrint();
	}

	// this alloc should go OOM!
	void *testptr = alloc.allocate(700);
	if(!testptr) {
		std::cerr << "OOM detected OK!" << std::endl;
	} else {
		std::cerr << "Failed to detect OOM!" << std::endl;
		return 1;
	}

	// but this one should work
	testptr = alloc.allocate(100);
	if(!testptr) {
		std::cerr << "OOM false-positive!" << std::endl;
		return 1;
	} else {
		std::cerr << "No OOM detected -> good!" << std::endl;
	}

	// !!! Pointer is not added to the list to test remaining block counter

	// delete some random blocks
	std::random_shuffle(ptrs.begin(), ptrs.end());

	for(int i = 0; i < 5; i++) {
		auto iter = ptrs.end() - 1;
		alloc.deallocate(*iter);
		ptrs.erase(iter);

		alloc.debugPrint();
	}

	// this alloc should go OOM!
	testptr = alloc.allocate(6*256+1);
	if(!testptr) {
		std::cerr << "OOM detected OK!" << std::endl;
	} else {
		std::cerr << "Failed to detect OOM!" << std::endl;
		return 1;
	}

	// some 2-block allocs should be possible with the above seed
	for(int i = 0; i < 6; i++) {
		void *ptr = alloc.allocate(512);
		assert(ptr);
		ptrs.push_back(ptr);
		alloc.debugPrint();
	}

	// delete some random blocks again
	std::random_shuffle(ptrs.begin(), ptrs.end());

	for(int i = 0; i < 10; i++) {
		auto iter = ptrs.end() - 1;
		alloc.deallocate(*iter);
		ptrs.erase(iter);

		alloc.debugPrint();
	}

	// allocate a block
	testptr = alloc.allocate(3*256);
	assert(testptr);
	alloc.debugPrint();

	// resize it
	void *testptr2 = alloc.reallocate(testptr, 4*256);
	assert(testptr == testptr2);
	alloc.debugPrint();
	testptr = testptr2;

	// resize it again -> this should relocate the block
	testptr2 = alloc.reallocate(testptr, 7*256);
	assert(testptr != testptr2);
	alloc.debugPrint();
	testptr = testptr2;

	// delete the block
	alloc.deallocate(testptr);
	alloc.debugPrint();

	// erase all remembered blocks
	for(void *ptr: ptrs) {
		alloc.deallocate(ptr);
		alloc.debugPrint();
	}

	std::cerr << "1 Block/Alloc should be remaining." << std::endl;
}
