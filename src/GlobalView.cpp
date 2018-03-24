#include "Field.h"
#include "Bot.h"
#include "Food.h"
#include "LocalView.h"

#include "config.h"

#include "GlobalView.h"

// Private methods

std::size_t GlobalView::hashMapEntryFromVector2D(const Vector2D &vec)
{
	return m_hashMapSizeX * static_cast<std::size_t>(vec.y() / config::GLOBALVIEW_GRID_UNIT) +
			static_cast<std::size_t>(vec.x() / config::GLOBALVIEW_GRID_UNIT);
}

void GlobalView::normalizeHashMapCoord(long *coord, long range) const
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

	// update the hash map sizes for the given field
	m_hashMapSizeX = static_cast<std::size_t>(field->getSize().x() / config::GLOBALVIEW_GRID_UNIT + 1);
	m_hashMapSizeY = static_cast<std::size_t>(field->getSize().y() / config::GLOBALVIEW_GRID_UNIT + 1);

	std::size_t elements = m_hashMapSizeX * m_hashMapSizeY;

	m_foodInfoHashMap.resize(elements);
	m_segmentInfoHashMap.resize(elements);

	for(std::size_t i = 0; i < elements; i++) {
		m_foodInfoHashMap[i].clear();
		m_segmentInfoHashMap[i].clear();
	}

	for(auto &f : field->getStaticFood()) {
		std::size_t hashMapEntry = hashMapEntryFromVector2D(f->pos());
		m_foodInfoHashMap[hashMapEntry].emplace_back(f);
	}

	for(auto &f : field->getDynamicFood()) {
		std::size_t hashMapEntry = hashMapEntryFromVector2D(f->pos());
		m_foodInfoHashMap[hashMapEntry].emplace_back(f);
	}

	for(auto &b : field->getBots()) {
		for(auto &s : b->getSnake()->getSegments()) {
			std::size_t hashMapEntry = hashMapEntryFromVector2D(s->pos());

			m_segmentInfoHashMap[hashMapEntry].emplace_back(s, b);
		}
	}
}

std::shared_ptr<LocalView> GlobalView::extractLocalView(const Vector2D &center, float_t radius) const
{
	if(!m_field) {
		return NULL;
	}

	std::shared_ptr<LocalView> localView = std::make_shared<LocalView>(m_field, center, radius);

	long hashMapCenterX = static_cast<long>(center.x() / config::GLOBALVIEW_GRID_UNIT + 0.5f);
	long hashMapCenterY = static_cast<long>(center.y() / config::GLOBALVIEW_GRID_UNIT + 0.5f);

	long hashMapDist = static_cast<long>(radius / config::GLOBALVIEW_GRID_UNIT) + 1;

	long hashMapStartX = hashMapCenterX - hashMapDist;
	long hashMapStartY = hashMapCenterY - hashMapDist;
	long hashMapEndX   = hashMapCenterX + hashMapDist + 1;
	long hashMapEndY   = hashMapCenterY + hashMapDist + 1;

	normalizeHashMapCoord(&hashMapStartX, static_cast<long>(m_hashMapSizeX));
	normalizeHashMapCoord(&hashMapStartY, static_cast<long>(m_hashMapSizeY));
	normalizeHashMapCoord(&hashMapEndX, static_cast<long>(m_hashMapSizeX));
	normalizeHashMapCoord(&hashMapEndY, static_cast<long>(m_hashMapSizeY));

	long x = hashMapStartX;
	while(x != hashMapEndX) {
		long y = hashMapStartY;
		while(y != hashMapEndY) {
			long hashMapEntry = m_hashMapSizeX * y + x;

			localView->appendFood(m_foodInfoHashMap[hashMapEntry]);
			localView->appendSegments(m_segmentInfoHashMap[hashMapEntry]);

			y++;
			if(y >= static_cast<long>(m_hashMapSizeY)) {
				y = 0;
			}
		}

		x++;
		if(x >= static_cast<long>(m_hashMapSizeX)) {
			x = 0;
		}
	}

	return localView;
}
