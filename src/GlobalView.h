#pragma once

#include <memory>
#include <vector>

#include "Snake.h"
#include "Food.h"
#include "SpatialMap.h"

// forward declaration
class Field;
class Bot;

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

			const Vector2D& pos() const { return segment->pos(); }
		};
		typedef SpatialMap<SnakeSegmentInfo, 128, 128> SegmentInfoMap;

		struct FoodInfo {
			std::shared_ptr<Food> food;
			FoodInfo(const std::shared_ptr<Food> &f) : food(f) {}
			const Vector2D& pos() const { return food->pos(); }
		};
		typedef SpatialMap<FoodInfo, 128, 128> FoodInfoMap;

	private:
		const Field& m_field;
		FoodInfoMap m_foodMap;
		SegmentInfoMap m_segmentInfoMap;

	public:
		/*!
		 * Initialize a new GlobalView object.
		 */
		GlobalView(const Field &field);

		/*!
		 * Rebuild the GlobalView
		 */
		void rebuild();

		const FoodInfoMap& getFoodInfoMap() const { return m_foodMap; }
		const SegmentInfoMap& getSegmentInfoMap() const { return m_segmentInfoMap; }
};
