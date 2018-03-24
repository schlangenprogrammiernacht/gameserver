#include "Field.h"

#include "LocalView.h"

LocalView::LocalView(const Field *field, const Vector2D &center, float_t radius)
	: m_field(field), m_center(center), m_radius(radius)
{
}

void LocalView::appendFood(const GlobalView::FoodInfoList &foodInfo)
{
	for(auto &f: foodInfo) {
		// create new element at the end of the list
		auto elem = m_foodInfo.emplace(m_foodInfo.end());

		elem->pos = m_field->unwrapCoords(f.food->pos, m_center);
		elem->food = f.food;
	}
}

void LocalView::appendSegments(const GlobalView::SnakeSegmentInfoList &segmentInfo)
{
	for(auto &s: segmentInfo) {
		// create new element at the end of the list
		auto elem = m_segmentInfo.emplace(m_segmentInfo.end());

		elem->pos = m_field->unwrapCoords(s.segment->pos, m_center);
		elem->bot = s.bot;
	}
}

const LocalView::SnakeSegmentInfoList& LocalView::getSnakeSegments(void)
{
	return m_segmentInfo;
}

const LocalView::FoodInfoList& LocalView::getFood(void)
{
	return m_foodInfo;
}
