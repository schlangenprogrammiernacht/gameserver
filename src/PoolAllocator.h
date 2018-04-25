#pragma once

#include <cstddef>
#include <cstdint>

#include <vector>
#include <map>

class PoolAllocator
{
	private:
		std::vector<uint8_t> m_pool;

		std::map<std::size_t, std::size_t> m_blockMap; //!< Allocated Block Map (maps block-index to length)

		std::size_t          m_numBlocks;
		std::size_t          m_blockSize;

		std::size_t m_curBlockIdx;

		// stats
		std::size_t m_currentUsage = 0;
		std::size_t m_maxUsage = 0;
		std::size_t m_numAllocs = 0;

		/*!
		 * Find the next allocated block, starting from a given block.
		 *
		 * An allocated block can consist of multiple unit blocks, depending on
		 * it’s size.
		 *
		 * When the maximum search distance is reached, startBlock + maxDistance is
		 * returned.
		 *
		 * \param startBlock    The block where to start the search.
		 * \param maxDistance   Maximum search distance.
		 * \returns             The index of the next allocation.
		 */
		std::size_t findNextAlloc(std::size_t startBlock, std::size_t maxDistance);

		/*!
		 * Find a continuous sequence of the given number of free blocks.
		 *
		 * \param blocks    Number of free blocks to search for.
		 * \returns         The index of the first free block or SIZE_MAX when
		 *                  requirement cannot be fulfilled.
		 */
		std::size_t findFreeBlockSequence(std::size_t blocks);

		/*!
		 * Calculate number of blocks from the given number of bytes.
		 */
		std::size_t bytesToBlocks(std::size_t bytes);

		/*!
		 * Get a pointer to the block with the given index.
		 */
		uint8_t* blockIdxToPtr(std::size_t block);

		/*!
		 * Get the block index of a given pointer.
		 */
		std::size_t ptrToBlockIdx(void *ptr);

		/*!
		 * Track a change in the overall pool usage.
		 *
		 * \param oldSize   The old size of the block.
		 * \param newSize   The new size of the block.
		 */
		void trackUsage(std::size_t oldSize, std::size_t newSize);

	public:
		PoolAllocator(std::size_t bytes, std::size_t blockSize);
		~PoolAllocator();

		void* allocate(std::size_t bytes);
		void* reallocate(void *ptr, std::size_t bytes);
		void  deallocate(void *ptr);

		void debugPrint(void);

		/*!
		 * This is the implementation of the lua_Alloc type.
		 *
		 * See http://www.lua.org/manual/5.3/manual.html#lua_Alloc for further
		 * details.
		 *
		 * \param ud        User-data pointer (here: a pointer to a PoolAllocator).
		 * \param ptr       The pointer to be managed.
		 * \param osize     Old size of the data block.
		 * \param nsize     New size of the data block.
		 * \returns         A pointer to the (re)allocated data block.
		 */
		static void* lua_allocator(void *ud, void *ptr, size_t osize, size_t nsize);
};
