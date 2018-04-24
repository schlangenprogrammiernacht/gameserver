#include <cassert>
#include <iostream>
#include <algorithm>

#include "PoolAllocator.h"

int main(void)
{
	srand(1337); // for reproducible results

	PoolAllocator alloc(16384, 256);

	alloc.debugPrint();

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

	// 8 2-block allocs should be possible with the above seed
	for(int i = 0; i < 8; i++) {
		void *ptr = alloc.allocate(512);
		assert(ptr);
		ptrs.push_back(ptr);
		alloc.debugPrint();
	}

	for(void *ptr: ptrs) {
		alloc.deallocate(ptr);
		alloc.debugPrint();
	}

	// 1 Block/Alloc should be remaining
}
