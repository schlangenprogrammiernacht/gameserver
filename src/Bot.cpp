#include "Field.h"

#include "Bot.h"

Bot::Bot(Field *field, const std::string &name, const Vector &startPos, float_t startHeading)
	: m_name(name), m_field(field), m_moveCounter(0)
{
	// TODO: random start coordinates
	m_snake = std::make_shared<Snake>(field, startPos, 5, startHeading);

	m_heading = rand() * 360.0 / RAND_MAX;
}

std::size_t Bot::move(void)
{
	if((++m_moveCounter % 200) == 0) {
		m_heading = rand() * 360.0 / RAND_MAX;
	}

	return m_snake->move(m_heading); // direction in degrees
}

std::shared_ptr<Snake> Bot::getSnake(void) const
{
	return m_snake;
}

const std::string& Bot::getName(void) const
{
	return m_name;
}
