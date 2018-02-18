#include "Field.h"

#include "Bot.h"

Bot::Bot(Field *field, const std::string &name)
	: m_name(name), m_field(field)
{
	// TODO: random start coordinates
	m_snake = std::make_shared<Snake>(field, Vector(10, 10), 5, -60);
}

void Bot::move(void)
{
	m_snake->move(120); // direction in degrees
}

std::shared_ptr<Snake> Bot::getSnake(void)
{
	return m_snake;
}
