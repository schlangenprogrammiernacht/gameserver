#include "LocalView.h"

LocalView::LocalView(Field *field, const Vector &center, float_t radius)
	: m_field(field), m_center(center), m_radius(radius),
	  m_mustRecalculateSnakeSegmentInfo(true), m_mustRecalculateFoodInfo(true)
{
}

const LocalView::SnakeSegmentInfoList& LocalView::getSnakeSegments(void)
{
	if(!m_mustRecalculateSnakeSegmentInfo) {
		return m_segmentInfo;
	}

	// TODO: update segmentInfo

	return m_segmentInfo;
}

const LocalView::FoodInfoList& LocalView::getFood(void)
{
	if(!m_mustRecalculateFoodInfo) {
		return m_foodInfo;
	}

	// TODO: update foodInfo

	return m_foodInfo;
}

void LocalView::forceRecalculation(void)
{
	m_mustRecalculateSnakeSegmentInfo = true;
	m_mustRecalculateFoodInfo = true;
}
