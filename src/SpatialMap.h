#pragma once
#include <array>
#include <vector>
#include <tuple>
#include <functional>
#include "types.h"

template <class T, size_t TILES_X, size_t TILES_Y> class SpatialMap
{
	public:
		SpatialMap(size_t fieldSizeX, size_t fieldSizeY, size_t reserveCount)
			: m_fieldSizeX(fieldSizeX)
			, m_fieldSizeY(fieldSizeY)
			, m_tileSizeX(fieldSizeX/TILES_X)
			, m_tileSizeY(fieldSizeY/TILES_Y)
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

		void addElement(const T& element)
		{
			getTileVectorForPosition(element.pos()).push_back(element);
		}

		Vector2D unwrapRelativePos(const Vector2D& relativePos) const
		{
			float_t x = fmod(relativePos.x(), m_fieldSizeX);
			float_t y = fmod(relativePos.y(), m_fieldSizeY);
			if (x > m_fieldSizeX/2) { x -= m_fieldSizeX; }
			if (x < (-(int)m_fieldSizeX/2)) { x += m_fieldSizeX; }
			if (y > m_fieldSizeY/2) { y -= m_fieldSizeY; }
			if (y < (-(int)m_fieldSizeY/2)) { y += m_fieldSizeY; }
			return Vector2D { x, y };
		}

		typedef std::function<bool (const T&)> ProcessCallback;
		void processElements(const Vector2D& center, float_t radius, ProcessCallback callback) const
		{
			const Vector2D topLeft = center - Vector2D { radius, radius };
			const Vector2D bottomRight = center + Vector2D { radius, radius };

			int x1 = static_cast<int>(topLeft.x() / m_tileSizeX);
			int y1 = static_cast<int>(topLeft.y() / m_tileSizeY);
			int x2 = static_cast<int>(bottomRight.x() / m_tileSizeX);
			int y2 = static_cast<int>(bottomRight.y() / m_tileSizeY);

			for (int y=y1; y<=y2; y++)
			{
				for (int x=x1; x<=x2; x++)
				{
					for (auto& item: getTileVector(x, y))
					{
						if (!callback(item))
						{
							return;
						}
					}
				}
			}
		}

	private:
		size_t m_fieldSizeX, m_fieldSizeY;
		float_t m_tileSizeX, m_tileSizeY;
		typedef std::vector<T> TileVector;
		std::array<TileVector, TILES_X*TILES_Y> m_tiles;

		const TileVector& getTileVector(int tileX, int tileY) const
		{
			return m_tiles[wrap<TILES_Y>(tileY)*TILES_X + wrap<TILES_X>(tileX)];
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

};
