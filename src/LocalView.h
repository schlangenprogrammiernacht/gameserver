#pragma once

#include <memory>
#include <vector>

#include "GlobalView.h"

#include "types.h"

// forward declaration
class Field;
class Food;
class Bot;

/*!
 * Local, simplified view of a field around given absolute coordinates. All
 * coordinates in the snapshot are unwrapped.
 *
 * This class is intended to speed up algorithms working in a small area of the
 * field (such as collision checking).
 */
class LocalView
{
	public:
		struct SnakeSegmentInfo {
			Vector2D pos; //!< Unwrapped coordinates
			std::shared_ptr<Bot> bot; //!< The bot this segment belongs to
		};

		struct FoodInfo {
			Vector2D pos; //!< Unwrapped coordinates
			std::shared_ptr<Food> food;
		};

		typedef std::vector<SnakeSegmentInfo> SnakeSegmentInfoList;
		typedef std::vector<FoodInfo> FoodInfoList;

	private:
		SnakeSegmentInfoList m_segmentInfo;
		FoodInfoList m_foodInfo;

		const Field *m_field;
		Vector2D   m_center;
		float_t  m_radius;

	public:
		/*!
		 * Initialize a new LocalView object.
		 *
		 * \param field   The Field this instance should work in.
		 * \param center  The center point of the local view.
		 * \param radius  The radius of the local view.
		 */
		LocalView(const Field *field, const Vector2D &center, float_t radius);

		/*!
		 * Append food extracted from a GlobalView.
		 *
		 * \param foodInfo   Food info from the GlobalView.
		 */
		void appendFood(const GlobalView::FoodInfoList &foodInfo);

		/*!
		 * Append segments extracted from a GlobalView.
		 *
		 * \param segmentInfo   Segment info from the GlobalView.
		 */
		void appendSegments(const GlobalView::SnakeSegmentInfoList &segmentInfo);

		/*!
		 * Get the snake segment info list.
		 *
		 * The snake segments are sorted by distance from the center point.
		 *
		 * This function caches the list in a private member variable of this
		 * class. As long as forceRecalculation() is not called, all calls except
		 * the first will use the cached result.
		 *
		 * \returns   The list of snake segments.
		 */
		const SnakeSegmentInfoList& getSnakeSegments(void);

		/*!
		 * Get the food info list.
		 *
		 * The food items are sorted by distance from the center point.
		 *
		 * This function caches the list in a private member variable of this
		 * class. As long as forceRecalculation() is not called, all calls except
		 * the first will use the cached result.
		 *
		 * \returns   The list of food items.
		 */
		const FoodInfoList& getFood(void);
};
