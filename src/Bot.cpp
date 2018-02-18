#include "Field.h"

#include "Bot.h"

Bot::Bot(Field *field, const std::string &name, const Vector &startPos, float_t startHeading)
	: m_name(name), m_field(field)
{
	// TODO: random start coordinates
	m_snake = std::make_shared<Snake>(field, startPos, 5, startHeading);
}

void Bot::move(void)
{
	m_snake->move(120); // direction in degrees
}

std::shared_ptr<Snake> Bot::getSnake(void)
{
	return m_snake;
}
