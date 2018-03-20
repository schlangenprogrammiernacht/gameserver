#include "GlobalView.h"

#include <algorithm>
#include "Field.h"
#include "Bot.h"
#include "Food.h"
#include "LocalView.h"
#include "config.h"


// Private methods

std::size_t GlobalView::hashMapEntryFromVector(const Vector &vec)
{
	return m_hashMapSizeX * static_cast<std::size_t>(vec.y() / config::GLOBALVIEW_GRID_UNIT) +
							static_cast<std::size_t>(vec.x() / config::GLOBALVIEW_GRID_UNIT);
}

void GlobalView::normalizeHashMapCoord(long *coord, std::size_t range) const
{
	while(*coord < 0) {
		*coord += range;
	}

	while(*coord >= range) {
		*coord -= range;
	}
}

// Public methods

GlobalView::GlobalView()
	: m_field(NULL)
{
}

void GlobalView::rebuild(const Field *field)
{
	m_field = field;

	// cleanup
	m_foodInfoHashMap.clear();
	m_segmentInfoHashMap.clear();

	// update the hash map sizes for the given field
	m_hashMapSizeX = static_cast<std::size_t>(field->getSize().x() / config::GLOBALVIEW_GRID_UNIT + 1);
	m_hashMapSizeY = static_cast<std::size_t>(field->getSize().y() / config::GLOBALVIEW_GRID_UNIT + 1);

	std::size_t elements = m_hashMapSizeX * m_hashMapSizeY;

	m_foodInfoHashMap.resize(elements);
	m_segmentInfoHashMap.resize(elements);

	for(auto &f : field->getStaticFood()) {
		std::size_t hashMapEntry = hashMapEntryFromVector(f->getPosition());

		auto elem = m_foodInfoHashMap[hashMapEntry].emplace(
				m_foodInfoHashMap[hashMapEntry].end());

		elem->food = f;
	}

	for(auto &f : field->getDynamicFood()) {
		std::size_t hashMapEntry = hashMapEntryFromVector(f->getPosition());

		auto elem = m_foodInfoHashMap[hashMapEntry].emplace(
				m_foodInfoHashMap[hashMapEntry].end());

		elem->food = f;
	}

	for(auto &b : field->getBots()) {
		for(auto &s : b->getSnake()->getSegments()) {
			std::size_t hashMapEntry = hashMapEntryFromVector(s->pos);

			auto elem = m_segmentInfoHashMap[hashMapEntry].emplace(
					m_segmentInfoHashMap[hashMapEntry].end());

			elem->bot = b;
			elem->segment = s;
		}
	}
}

std::unique_ptr<LocalView> GlobalView::extractLocalView(const Vector &center, float_t radius) const
{
	if(!m_field) {
		return nullptr;
	}

	auto localView = std::make_unique<LocalView>(m_field, center, radius);

	long hashMapCenterX = static_cast<long>(center.x() / config::GLOBALVIEW_GRID_UNIT + 0.5f);
	long hashMapCenterY = static_cast<long>(center.y() / config::GLOBALVIEW_GRID_UNIT + 0.5f);

	long hashMapDist = static_cast<long>(radius / config::GLOBALVIEW_GRID_UNIT) + 1;

	long hashMapStartX = hashMapCenterX - hashMapDist;
	long hashMapStartY = hashMapCenterY - hashMapDist;
	long hashMapEndX   = hashMapCenterX + hashMapDist + 1;
	long hashMapEndY   = hashMapCenterY + hashMapDist + 1;

	normalizeHashMapCoord(&hashMapStartX, m_hashMapSizeX);
	normalizeHashMapCoord(&hashMapStartY, m_hashMapSizeY);
	normalizeHashMapCoord(&hashMapEndX, m_hashMapSizeX);
	normalizeHashMapCoord(&hashMapEndY, m_hashMapSizeY);

	long x = hashMapStartX;
	while(x != hashMapEndX) {
		long y = hashMapStartY;
		while(y != hashMapEndY) {
			long hashMapEntry = m_hashMapSizeX * y + x;

			localView->appendFood(m_foodInfoHashMap[hashMapEntry]);
			localView->appendSegments(m_segmentInfoHashMap[hashMapEntry]);

			y++;
			if(y >= m_hashMapSizeY) {
				y = 0;
			}
		}

		x++;
		if(x >= m_hashMapSizeX) {
			x = 0;
		}
	}

	return localView;
}

size_t GlobalView::normalize(int v, size_t max)
{
	v %= max;
	if (v<0) { v += max; }
	return static_cast<size_t>(v);
}

void GlobalView::findFood(const Vector &center, float_t radius, FoodCallback callback) const
{
	const int x1 = static_cast<int>(floor((center.x()-radius) / config::GLOBALVIEW_GRID_UNIT));
	const int x2 = static_cast<int>(ceil((center.x()+radius) / config::GLOBALVIEW_GRID_UNIT));
	const int y1 = static_cast<int>(floor((center.y()-radius) / config::GLOBALVIEW_GRID_UNIT));
	const int y2 = static_cast<int>(ceil((center.y()+radius) / config::GLOBALVIEW_GRID_UNIT));
	const int x_start = std::min(x1, x2);
	const int x_end = std::max(x1, x2);
	const int y_start = std::min(y1, y2);
	const int y_end = std::max(y1, y2);

	const float squaredRadius = radius*radius;

	for (int y=y_start; y<=y_end; y++)
	{
		for (int x=x_start; x<x_end; x++)
		{
			size_t key = m_hashMapSizeX * normalize(y, m_hashMapSizeY) + normalize(x, m_hashMapSizeX);
			for (auto& item: m_foodInfoHashMap[key])
			{
				Vector pos = item.food->getPosition()-center;
				Vector wrappedPos {
					fmodf(pos.x() + config::FIELD_SIZE_X/2, config::FIELD_SIZE_X) - config::FIELD_SIZE_X/2,
					fmodf(pos.y() + config::FIELD_SIZE_Y/2, config::FIELD_SIZE_Y) - config::FIELD_SIZE_Y/2,
				};
				if (wrappedPos.squaredAbs() <= squaredRadius)
				{
					callback(pos, item);
				}
			}
		}
	}
}
