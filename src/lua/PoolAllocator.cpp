/*
 * Schlangenprogrammiernacht: A programming game for GPN18.
 * Copyright (C) 2018  bytewerk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

std::size_t PoolAllocator::findNextAlloc(std::size_t startBlock, std::size_t maxDistance)
{
	std::size_t i = startBlock;
	while((i < m_numBlocks) && (i < startBlock + maxDistance)) {
		if(m_blockMap.find(i) != m_blockMap.end()) {
			// found a block starting at this index
			break;
		}

		++i;
	}

	return i;
}

std::size_t PoolAllocator::findFreeBlockSequence(std::size_t blocks)
{
	std::size_t startBlock = m_curBlockIdx;
	std::size_t totalInc = 0;

	while(totalInc < m_numBlocks) {
		if(startBlock >= m_numBlocks) {
			startBlock = 0;
		}

		std::size_t nextAlloc = findNextAlloc(startBlock, blocks+1);
		std::size_t nextAllocDistance = nextAlloc - startBlock;

		totalInc += nextAllocDistance;

		if(nextAllocDistance >= blocks) {
			return startBlock;
		} else {
			startBlock = nextAlloc;

			std::map<std::size_t, std::size_t>::iterator iter;
			while((totalInc < m_numBlocks) &&
					(iter = m_blockMap.find(startBlock)) != m_blockMap.end()) {
				startBlock += iter->second;
				totalInc += iter->second;
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

	if(startBlock == SIZE_MAX) {
		PA_DEBUG(std::cerr << "PoolAllocator: could not find " << blocks << " contiguous free blocks :(" << std::endl);
		// out of memory :(
		return nullptr;
	}

	assert(startBlock <= (m_numBlocks - blocks));

	PA_DEBUG(std::cerr << "PoolAllocator: found free block at index " << startBlock << std::endl);

	// remember block size
	m_blockMap[startBlock] = blocks;

	// set next search index for faster allocation
	m_curBlockIdx = startBlock;

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
		m_blockMap[origStartBlock] = newBlocks;

		trackUsage(oldBlocks, newBlocks);

		return ptr;
	} else {
		// block should be grown

		std::size_t nextAlloc = findNextAlloc(origStartBlock + 1, newBlocks);
		std::size_t nextAllocDistance = nextAlloc - origStartBlock;

		if(nextAllocDistance >= newBlocks) {
			PA_DEBUG(std::cerr << "PoolAllocator: Reallocation: found enough free blocks after the current block." << std::endl);
			assert(origStartBlock <= (m_numBlocks - newBlocks));

			// increase mapped block size
			m_blockMap[origStartBlock] = newBlocks;

			trackUsage(oldBlocks, newBlocks);

			// play it safe and prevent growing over the curBlockIdx
			m_curBlockIdx = 0;

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

	PA_DEBUG(std::cerr << "PoolAllocator: Freeing block " << block << " with size " << length << " blocks" << std::endl);

	m_blockMap.erase(block);

	trackUsage(length, 0);
}

void PoolAllocator::debugPrint(void)
{
	std::vector<std::size_t> usageMap;
	usageMap.resize(m_numBlocks, 0);

	for(auto &elem: m_blockMap) {
		for(std::size_t i = 0; i < elem.second; i++) {
			usageMap[elem.first + i]++;
		}
	}

	std::cerr << "Usage map: ";
	for(auto &e: usageMap) {
		if(e == 0) {
			std::cerr << ".";
		} else if(e < 10) {
			std::cerr << e;
		} else {
			std::cerr << "#";
		}
	}
	std::cerr << std::endl;
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
