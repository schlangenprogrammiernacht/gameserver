#pragma once
#include <array>
#include <vector>
#include "types.h"

template <class T, size_t TILES_X, size_t TILES_Y> class SpatialMap
{
	public:
		SpatialMap(size_t fieldSizeX, size_t fieldSizeY)
			: m_fieldSizeX(fieldSizeX)
			, m_fieldSizeY(fieldSizeY)
		{}

	private:
		typedef std::vector<T> TileVector;

		std::array<TileVector, TILES_X*TILES_Y> m_tiles;
		size_t m_fieldSizeX, m_fieldSizeY;
};
