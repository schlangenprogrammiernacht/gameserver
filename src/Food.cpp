#include "config.h"

#include "Food.h"

Food::Food(bool shallRegenerate, const Vector2D &pos, real_t value,
		const std::shared_ptr<Bot> &hunter)
	: PositionObject(pos)
	, m_value(value)
	, m_shallRegenerate(shallRegenerate)
	, m_shallBeRemoved(false)
	, m_hunter(hunter)
{
}

bool Food::decay(void)
{
	m_value -= config::FOOD_DECAY_STEP;
	m_shallBeRemoved |= m_value <= 0;
	return m_shallBeRemoved;
}

bool Food::hasDecayed(void) const
{
	return m_value <= 0;
}
