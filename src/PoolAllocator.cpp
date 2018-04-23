#include <cassert>
#include <cstring>
#include <iostream>

#include "PoolAllocator.h"

#define DEBUG_POOLALLOCATOR_STATS
//#define DEBUG_POOLALLOCATOR

#ifdef DEBUG_POOLALLOCATOR
#define PA_DEBUG(x) x
#else
#define PA_DEBUG(x)
#endif

/* Private methods */

std::size_t PoolAllocator::findFreeBlockSequence(std::size_t blocks)
{
	std::size_t startBlock = m_curBlockIdx;
	std::size_t lastBlockToCheck = (m_curBlockIdx + m_numBlocks - 1) % m_numBlocks;

	while(startBlock != lastBlockToCheck) {
		bool rangeFree = true;

		std::size_t offset;
		for(offset = 0; offset < blocks; offset++) {
			std::size_t idx = startBlock + offset;

			if(idx >= m_numBlocks) {
				rangeFree = false;
				break;
			}

			if(idx == lastBlockToCheck) {
				/* Came across the last block to check, which means there is not enough
				 * memory left for this allocation. Panic? */
				return SIZE_MAX;
			}

			if(m_blockUsed[idx]) {
				rangeFree = false;
				break;
			}
		}

		if(rangeFree) {
			return startBlock;
		} else {
			startBlock += offset + 1;
			if(startBlock >= m_numBlocks) {
				startBlock -= m_numBlocks;
			}
		}
	}

	/* Checked the whole pool. */
	return SIZE_MAX;
}

std::size_t PoolAllocator::bytesToBlocks(std::size_t bytes)
{
	std::size_t blocks = bytes / m_blockSize;

	if((bytes % m_blockSize) != 0) {
		blocks++;
	}

	return blocks;
}

uint8_t* PoolAllocator::blockIdxToPtr(std::size_t block)
{
	return m_pool.data() + (m_blockSize * block);
}

std::size_t PoolAllocator::ptrToBlockIdx(void *ptr)
{
	return static_cast<std::size_t>(reinterpret_cast<uint8_t*>(ptr) - m_pool.data()) / m_blockSize;
}

void PoolAllocator::trackUsage(std::size_t oldSize, std::size_t newSize)
{
	m_currentUsage = m_currentUsage + newSize - oldSize;

	if(m_currentUsage > m_maxUsage) {
		m_maxUsage = m_currentUsage;
	}
}

/* Public methods */

PoolAllocator::PoolAllocator(std::size_t bytes, std::size_t blockSize)
	: m_blockSize(blockSize)
{
	m_numBlocks = bytes / m_blockSize;
	if((bytes % m_blockSize) != 0) {
		m_numBlocks++;
	}

	m_pool.resize(m_numBlocks * m_blockSize);
	m_blockUsed.resize(m_numBlocks, false); // all blocks unused at startup

	m_curBlockIdx = 0;
}

PoolAllocator::~PoolAllocator()
{
#ifdef DEBUG_POOLALLOCATOR_STATS
	std::cerr << "PoolAllocator: Destroying pool: remaining memory: " << m_currentUsage << " blocks; Max: " << m_maxUsage << " blocks" << std::endl;
	std::cerr << "PoolAllocator: Destroying pool: Number of allocations: " << m_numAllocs << std::endl;
#endif // DEBUG_POOLALLOCATOR_STATS
}

void* PoolAllocator::allocate(std::size_t bytes)
{
	std::size_t blocks = bytesToBlocks(bytes);

	PA_DEBUG(std::cerr << "PoolAllocator: allocating block with " << bytes << " bytes/" << blocks << " blocks." << std::endl);

	std::size_t startBlock = findFreeBlockSequence(blocks);

	assert(startBlock <= (m_numBlocks - blocks));

	if(startBlock == SIZE_MAX) {
		PA_DEBUG(std::cerr << "PoolAllocator: could not find " << blocks << " contiguous free blocks :(" << std::endl);
		// out of memory :(
		return nullptr;
	}

	PA_DEBUG(std::cerr << "PoolAllocator: found free block at index " << startBlock << std::endl);

	// mark blocks as allocated
	for(std::size_t i = 0; i < blocks; i++) {
		m_blockUsed[startBlock + i] = true;
	}

	// remember block size
	m_blockMap[startBlock] = blocks;

	// set next search index for faster allocation
	m_curBlockIdx += blocks;
	if(m_curBlockIdx >= m_numBlocks) {
		m_curBlockIdx = 0;
	}

	trackUsage(0, blocks);
	m_numAllocs++;

	return blockIdxToPtr(startBlock);
}

void* PoolAllocator::reallocate(void *ptr, std::size_t bytes)
{
	if(!ptr) {
		return allocate(bytes);
	}

	std::size_t origStartBlock = ptrToBlockIdx(ptr);

	assert(origStartBlock < m_numBlocks);

	std::size_t newBlocks = bytesToBlocks(bytes);
	std::size_t oldBlocks = m_blockMap[origStartBlock];

	PA_DEBUG(std::cerr << "PoolAllocator: Resizing block " << origStartBlock << " from " << oldBlocks << " to " << newBlocks << " blocks" << std::endl);

	if(oldBlocks == newBlocks) {
		// nothing to do
		return ptr;
	} else if(newBlocks < oldBlocks) {
		// block shrinked -> free now unused blocks
		for(size_t i = oldBlocks; i < newBlocks; i++) {
			m_blockUsed[origStartBlock + i] = false;
		}

		m_blockMap[origStartBlock] = newBlocks;

		trackUsage(oldBlocks, newBlocks);

		return ptr;
	} else {
		// block should be grown

		// check if there are enough free blocks following
		bool reallocated = true;

		if((origStartBlock + newBlocks) < m_numBlocks) {
			for(std::size_t i = oldBlocks; i < newBlocks; i++) {
				std::size_t blockIdx = origStartBlock + i;

				if(m_blockUsed[blockIdx]) {
					// found a used block in our new block range,
					// so we cannot simply grow the block
					reallocated = false;
					break;
				}
			}
		} else {
			// block would grow over the end of buffer
			reallocated = false;
		}

		if(reallocated) {
			PA_DEBUG(std::cerr << "PoolAllocator: Reallocation: found enough free blocks after the current block." << std::endl);

			// mark blocks as used
			for(std::size_t i = oldBlocks; i < newBlocks; i++) {
				std::size_t blockIdx = origStartBlock + i;
				m_blockUsed[blockIdx] = true;
			}
			assert(origStartBlock <= (m_numBlocks - newBlocks));

			// increase mapped block size
			m_blockMap[origStartBlock] = newBlocks;

			trackUsage(oldBlocks, newBlocks);

			// block pointer has not changed
			return ptr;
		} else {
			PA_DEBUG(std::cerr << "PoolAllocator: Reallocation: moving memory to a new block." << std::endl);

			// try to find a new block and copy the memory there
			void *newBlockPtr = allocate(newBlocks*m_blockSize);

			if(!newBlockPtr) {
				return nullptr;
			}

			std::memcpy(newBlockPtr, ptr, oldBlocks*m_blockSize);

			deallocate(ptr);

			return newBlockPtr;
		}
	}
}

void PoolAllocator::deallocate(void *ptr)
{
	if(!ptr) {
		return;
	}

	std::size_t block = ptrToBlockIdx(ptr);
	std::size_t length = m_blockMap[block];

	assert(block < m_numBlocks);

	for(size_t i = 0; i < length; i++) {
		m_blockUsed[block + i] = false;
	}

	m_blockMap.erase(block);

	trackUsage(length, 0);
}

void* PoolAllocator::lua_allocator(void *ud, void *ptr, size_t osize, size_t nsize)
{
	PoolAllocator *allocator = reinterpret_cast<PoolAllocator*>(ud);

	// free the block if new size is 0
	if(nsize == 0) {
		allocator->deallocate(ptr);
		return nullptr;
	}

	// else (re)allocate the block
	return allocator->reallocate(ptr, nsize);
}
