#include <iostream>
#include <vector>
#include <algorithm>

#include "Field.h"

Field::Field()
	: m_width(128), m_height(128)
{
}

Field::Field(float_t w, float_t h)
	: m_width(w), m_height(h)
{
}

void Field::newBot(const std::string &name)
{
	std::shared_ptr<Bot> bot = std::make_shared<Bot>(this, name);

	m_bots.insert(bot);
}

void Field::moveAllBots(void)
{
	for(auto &b: m_bots) {
		b->move();
	}
}

const Field::BotSet& Field::getBots(void)
{
	return m_bots;
}

Vector Field::wrapCoords(const Vector &v)
{
	Vector result(v);

	while(result.x() < 0) {
		result.set_x(result.x() + m_width);
	}

	while(result.x() > m_width) {
		result.set_x(result.x() - m_width);
	}

	while(result.y() < 0) {
		result.set_y(result.y() + m_height);
	}

	while(result.y() > m_height) {
		result.set_y(result.y() - m_height);
	}

	return result;
}

Vector Field::unwrapCoords(const Vector &v, const Vector &ref)
{
	Vector result(v);

	while((result.x() - ref.x()) < -m_width/2) {
		result.set_x(result.x() + m_width);
	}

	while((result.x() - ref.x()) > m_width/2) {
		result.set_x(result.x() - m_width);
	}

	while((result.y() - ref.y()) < -m_height/2) {
		result.set_y(result.y() + m_height);
	}

	while((result.y() - ref.y()) > m_height/2) {
		result.set_y(result.y() - m_height);
	}

	return result;
}

void Field::debugVisualization(void)
{
	size_t intW = static_cast<size_t>(m_width);
	size_t intH = static_cast<size_t>(m_height);

	std::vector<char> rep(intW*intH);

	std::fill(rep.begin(), rep.end(), '.');

	for(auto &b: m_bots) {
		std::shared_ptr<Snake> snake = b->getSnake();

		bool first = true;
		for(auto &seg: snake->getSegments()) {
			size_t x = static_cast<size_t>(seg->pos.x());
			size_t y = static_cast<size_t>(seg->pos.y());

			if(first) {
				rep[y*intW + x] = '#';
				first = false;
			} else {
				rep[y*intW + x] = '+';
			}
		}
	}

	for(std::size_t y = 0; y < intH; y++) {
		for(std::size_t x = 0; x < intW; x++) {
			std::cout << rep[y*intW + x];
		}

		std::cout << "\n";
	}

	std::cout << std::endl;
}
