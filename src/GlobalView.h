#pragma once

#include <memory>
#include <vector>

#include "Snake.h"
#include "SpatialMap.h"

// forward declaration
class Field;
class Food;
class Bot;

class LocalView;

/*!
 * Global, simplified view of the field.
 *
 * This class contains all objects on the field, sorted into a grid by their
 * coordinates. The grid acts as a hashmap for quick access to objects in a
 * certain coordinate range.
 */
class GlobalView
{
	public:
		struct SnakeSegmentInfo {
			std::shared_ptr<Snake::Segment> segment; //!< Pointer to the segment
			std::shared_ptr<Bot> bot; //!< The bot this segment belongs to

			SnakeSegmentInfo(const std::shared_ptr<Snake::Segment> &s, const std::shared_ptr<Bot> &b)
				: segment(s), bot(b) {}

			const Vector2D& pos() { return segment->pos(); }
		};

		struct FoodInfo {
			std::shared_ptr<Food> food;
			FoodInfo(const std::shared_ptr<Food> &f) : food(f) {}
		};

		typedef std::vector<SnakeSegmentInfo> SnakeSegmentInfoList;
		typedef std::vector<FoodInfo> FoodInfoList;

	private:
		SpatialMap<Food, 128, 128> m_foodMap;
		SpatialMap<SnakeSegmentInfo, 128, 128> m_segmentInfoMap;

		std::vector<SnakeSegmentInfoList> m_segmentInfoHashMap;
		std::vector<FoodInfoList>         m_foodInfoHashMap;

		std::size_t m_hashMapSizeX;
		std::size_t m_hashMapSizeY;

		const Field *m_field;

		std::size_t hashMapEntryFromVector2D(const Vector2D &vec);

		void normalizeHashMapCoord(long *coord, long range) const;

	public:
		/*!
		 * Initialize a new GlobalView object.
		 */
		GlobalView(const Field *field);

		/*!
		 * Rebuild the GlobalView from the given Field.
		 */
		void rebuild();

		/*!
		 * Create a LocalView object around the given coordinates from this
		 * GlobalView.
		 *
		 * \param center    Center of the LocalView.
		 * \param radius    Radius of the LocalView.
		 * \returns         A shared pointer to the new LocalView object.
		 */
		std::shared_ptr<LocalView> extractLocalView(const Vector2D &center, float_t radius) const;
};
