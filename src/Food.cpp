#include "config.h"

#include "Food.h"

Food::Food(Field *field, const Vector &pos, float_t value)
	: m_field(field), m_pos(pos), m_value(value)
{
}

bool Food::canBeEatenBy(const std::shared_ptr<Snake> &snake) const
{
	const Snake::SegmentList &snakeSegments = snake->getSegments();

	float_t distance = m_pos.distanceTo(snakeSegments[0]->pos);

	return distance < config::SNAKE_EATING_RADIUS;
}
