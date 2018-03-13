#include "Field.h"
#include "Bot.h"
#include "Food.h"
#include "LocalView.h"

#include "config.h"

#include "GlobalView.h"

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

std::shared_ptr<LocalView> GlobalView::extractLocalView(const Vector &center, float_t radius) const
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
			if(y > m_hashMapSizeY) {
				y = 0;
			}
		}

		x++;
		if(x > m_hashMapSizeX) {
			x = 0;
		}
	}

	return localView;
}
