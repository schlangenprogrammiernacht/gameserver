#pragma once
#include <array>
#include <vector>
#include <functional>
#include "types.h"

template <class T> class SpatialMapRegion;

template <class T, size_t TILES_X, size_t TILES_Y> class SpatialMap
{
	public:
		typedef std::vector<T> TileVector;
		typedef SpatialMapRegion<SpatialMap<T,TILES_X,TILES_Y>> Region;
		friend class SpatialMapRegion<SpatialMap<T,TILES_X,TILES_Y>>;

	public:
		SpatialMap(size_t fieldSizeX, size_t fieldSizeY, size_t reserveCount)
			: m_fieldSizeX(fieldSizeX)
			, m_fieldSizeY(fieldSizeY)
			, m_tileSizeX(fieldSizeX/TILES_X)
			, m_tileSizeY(fieldSizeY/TILES_Y)
			, m_fullRegion(*this, 0, 0, TILES_X-1, TILES_Y-1)
		{
			for (auto &v: m_tiles)
			{
				v.reserve(reserveCount);
			}
		}

		void clear()
		{
			for (auto &v: m_tiles)
			{
				v.clear();
			}
		}

		size_t size() const
		{
			size_t retval = 0;
			for (auto &tile: m_tiles)
			{
				retval += tile.size();
			}
			return retval;
		}

		void addElement(const T& element)
		{
			getTileVectorForPosition(element.pos()).push_back(element);
		}

		void erase_if(std::function<bool(const T&)> predicate)
		{
			for (auto &tile: m_tiles)
			{
				tile.erase(std::remove_if(tile.begin(), tile.end(), predicate), tile.end());
			}
		}

		Region getRegion(const Vector2D& center, real_t radius)
		{
			const Vector2D topLeft = center - Vector2D { radius, radius };
			const Vector2D bottomRight = center + Vector2D { radius, radius };
			return {
				*this,
				static_cast<int>(topLeft.x() / m_tileSizeX),
				static_cast<int>(topLeft.y() / m_tileSizeY),
				static_cast<int>(bottomRight.x() / m_tileSizeX),
				static_cast<int>(bottomRight.y() / m_tileSizeY)
			};
		}

		typename Region::Iterator begin()
		{
			return m_fullRegion.begin();
		}

		typename Region::Iterator end()
		{
			return m_fullRegion.end();
		}

	private:
		size_t m_fieldSizeX, m_fieldSizeY;
		real_t m_tileSizeX, m_tileSizeY;
		Region m_fullRegion;
		std::array<TileVector, TILES_X*TILES_Y> m_tiles;

		TileVector& getTileVector(int tileX, int tileY)
		{
			return m_tiles[wrap<TILES_Y>(tileY)*TILES_X + wrap<TILES_X>(tileX)];
		}

		TileVector& getTileVectorNoWrap(int tileX, int tileY)
		{
			return m_tiles[tileY*TILES_X +tileX];
		}

		TileVector& getTileVectorForPosition(const Vector2D& pos)
		{
			size_t tileX = wrap<TILES_X>(pos.x() / m_tileSizeX);
			size_t tileY = wrap<TILES_Y>(pos.y() / m_tileSizeY);
			return m_tiles[tileY*TILES_X + tileX];
		}

		template <size_t SIZE> static size_t wrap(int unwrapped)
		{
			int result = (unwrapped % SIZE);
			if (result<0) { result += SIZE; }
			return static_cast<size_t>(result);
		}

		static bool needsWrapX(int unwrapped)
		{
			return (unwrapped<0) || (unwrapped>=TILES_X);
		}

		static bool needsWrapY(int unwrapped)
		{
			return (unwrapped<0) || (unwrapped>=TILES_Y);
		}

};

template <class T> class SpatialMapRegion
{
	public:

		class Iterator
		{
			public:
				Iterator(SpatialMapRegion* region, bool atEnd)
					: m_region(region)
					, m_needsWrap(region->m_needsWrap)
					, m_atEnd(atEnd)
				{
					if (m_atEnd)
					{
						moveToEnd();
					}
					else
					{
						m_tileX = m_region->m_x1;
						m_tileY = m_region->m_y1;
						skipEmptyTiles();
					}
				}

				Iterator& operator++()
				{
					if (m_atEnd) { return *this; }
					if (++m_positionInTile >= getCurrentTile().size())
					{
						m_positionInTile = 0;
						m_tileX++;
						skipEmptyTiles();
					}
					return *this;
				}

				bool operator!=(const Iterator& other)
				{
					if (m_atEnd)
					{
						return (other.m_region != m_region) || (!other.m_atEnd);
					}

					return (other.m_region != m_region)
						|| (other.m_atEnd!=m_atEnd)
						|| (other.m_tileX != m_tileX)
						|| (other.m_tileY != m_tileY)
						|| (other.m_positionInTile != m_positionInTile);
				}

				auto& operator*()
				{
					return getCurrentTile()[m_positionInTile];
				}

			private:
				SpatialMapRegion* m_region;
				const bool m_needsWrap = true;
				bool m_atEnd = false;
				int m_tileX = 0;
				int m_tileY = 0;
				size_t m_positionInTile = 0;

				void skipEmptyTiles()
				{
					while (m_tileY <= m_region->m_y2)
					{
						while (m_tileX <= m_region->m_x2)
						{
							if (getCurrentTile().size()>0)
							{
								return;
							}
							m_tileX++;
						}
						m_tileX = m_region->m_x1;
						m_tileY++;
					}
					moveToEnd();
				}

				void moveToEnd()
				{
					m_atEnd = true;
					m_tileX = 0;
					m_tileY = 0;
					m_positionInTile = 0;
				}

				typename T::TileVector& getCurrentTile()
				{
					return m_needsWrap
						 ? m_region->m_map.getTileVector(m_tileX, m_tileY)
						 : m_region->m_map.getTileVectorNoWrap(m_tileX, m_tileY);
				}
		};

		SpatialMapRegion(T& map, int x1, int y1, int x2, int y2)
			: m_map(map), m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2)
			, m_needsWrap(map.needsWrapX(x1) || map.needsWrapX(x2) || map.needsWrapY(y1) || map.needsWrapY(y2))
		{
		}

		Iterator begin()
		{
			return Iterator(this, false);
		}

		Iterator end()
		{
			return Iterator(this, true);
		}

	private:
		T& m_map;
		const int m_x1, m_y1, m_x2, m_y2;
		const bool m_needsWrap;

};

