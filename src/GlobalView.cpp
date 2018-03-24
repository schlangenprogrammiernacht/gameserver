#include "GlobalView.h"

#include "Field.h"
#include "Bot.h"
#include "config.h"

GlobalView::GlobalView(const Field &field)
	: m_field(field)
	, m_foodMap(static_cast<size_t>(field.getSize().x()), static_cast<size_t>(field.getSize().y()), 100)
	, m_segmentInfoMap(static_cast<size_t>(field.getSize().x()), static_cast<size_t>(field.getSize().y()), 100)
{
}

void GlobalView::rebuild()
{
	m_foodMap.clear();
	m_segmentInfoMap.clear();

	for (auto &f : m_field.getStaticFood())
	{
		m_foodMap.addElement(f);
	}

	for (auto &f : m_field.getDynamicFood())
	{
		m_foodMap.addElement(f);
	}

	for (auto &b : m_field.getBots())
	{
		for(auto &s : b->getSnake()->getSegments())
		{
			m_segmentInfoMap.addElement({s, b});
		}
	}
}
