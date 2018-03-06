#include <iostream>
#include <vector>
#include <algorithm>

#include "config.h"

#include "Field.h"

Field::Field(float_t w, float_t h, std::size_t food_parts,
				const std::shared_ptr<UpdateTracker> &update_tracker)
	: m_width(w), m_height(h), m_updateTracker(update_tracker)
{
	setupRandomness();

	createStaticFood(food_parts);
}

void Field::createStaticFood(std::size_t count)
{
	for(std::size_t i = 0; i < count; i++) {
		float_t value = (*m_foodSizeDistribution)(*m_rndGen);
		float_t x     = (*m_positionXDistribution)(*m_rndGen);
		float_t y     = (*m_positionYDistribution)(*m_rndGen);

		std::shared_ptr<Food> newFood =
			std::make_shared<Food>(this, Vector(x, y), value);

		m_updateTracker->foodSpawned(newFood);
		m_staticFood.insert( newFood );
	}
}

void Field::setupRandomness(void)
{
	std::random_device rd;
	m_rndGen = std::make_unique<std::mt19937>(rd());

	m_foodSizeDistribution = std::make_unique< std::normal_distribution<float_t> >(
			config::FOOD_SIZE_MEAN, config::FOOD_SIZE_STDDEV);

	m_positionXDistribution =
		std::make_unique< std::uniform_real_distribution<float_t> >(0, m_width);
	m_positionYDistribution =
		std::make_unique< std::uniform_real_distribution<float_t> >(0, m_height);

	m_headingDistribution =
		std::make_unique< std::uniform_real_distribution<float_t> >(-180, 180);
}

void Field::newBot(const std::string &name)
{
	float_t x = (*m_positionXDistribution)(*m_rndGen);
	float_t y = (*m_positionYDistribution)(*m_rndGen);
	float_t heading = (*m_headingDistribution)(*m_rndGen);

	std::shared_ptr<Bot> bot = std::make_shared<Bot>(this, name, Vector(x,y), heading);

	std::cerr << "Created Bot with ID " << bot->getGUID() << std::endl;

	m_updateTracker->botSpawned(bot);

	m_bots.insert(bot);
}

void Field::updateFood(void)
{
	// step 1: handle static food.
	// when static food decays, it is recreated at different coordinates
	std::size_t foodToGenerate = 0;

	auto sfi = m_staticFood.begin();
	while(sfi != m_staticFood.end()) {
		(*sfi)->decay();
		if((*sfi)->hasDecayed()) {
			m_updateTracker->foodDecayed(*sfi);
			sfi = m_staticFood.erase(sfi);
			foodToGenerate++;
		} else {
			sfi++;
		}
	}

	createStaticFood(foodToGenerate);

	// step 2: handle dynamic food
	// when dynamic food decays, it is removed permanently
	auto dfi = m_dynamicFood.begin();
	while(dfi != m_dynamicFood.end()) {
		(*dfi)->decay();
		if((*dfi)->hasDecayed()) {
			m_updateTracker->foodDecayed(*dfi);
			dfi = m_staticFood.erase(dfi);
		} else {
			dfi++;
		}
	}
}

void Field::consumeFood(void)
{
	for(auto &b: m_bots) {
		// step 1: handle static food.
		// when static food is consumed, it is recreated at different coordinates
		std::size_t foodToGenerate = 0;

		auto sfi = m_staticFood.begin();
		while(sfi != m_staticFood.end()) {
			if(b->getSnake()->canConsume(*sfi)) {
				b->getSnake()->consume(*sfi);
				m_updateTracker->foodConsumed(*sfi, b);
				sfi = m_staticFood.erase(sfi);
				foodToGenerate++;
			} else {
				sfi++;
			}
		}

		createStaticFood(foodToGenerate);

		// step 2: handle dynamic food
		// when dynamic food is consumed, it is removed permanently
		auto dfi = m_dynamicFood.begin();
		while(dfi != m_dynamicFood.end()) {
			if(b->getSnake()->canConsume(*dfi)) {
				b->getSnake()->consume(*dfi);
				m_updateTracker->foodConsumed(*dfi, b);
				dfi = m_staticFood.erase(dfi);
			} else {
				dfi++;
			}
		}
	}
}

void Field::moveAllBots(void)
{
	for(auto &b: m_bots) {
		size_t steps = b->move();
		m_updateTracker->botMoved(b, steps);
	}
}

const Field::BotSet& Field::getBots(void) const
{
	return m_bots;
}

const Field::FoodSet& Field::getStaticFood(void) const
{
	return m_staticFood;
}

const Field::FoodSet& Field::getDynamicFood(void) const
{
	return m_dynamicFood;
}

Vector Field::wrapCoords(const Vector &v) const
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

Vector Field::unwrapCoords(const Vector &v, const Vector &ref) const
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

	// empty cells are dots
	std::fill(rep.begin(), rep.end(), '.');

	// draw food
	for(auto &f: m_staticFood) {
		const Vector &pos = f->getPosition();

		char c;

		if(f->getValue() > 10) {
			c = 'X';
		} else {
			c = '0' + static_cast<int>(f->getValue());
		}

		size_t x = static_cast<size_t>(pos.x());
		size_t y = static_cast<size_t>(pos.y());

		rep[y * intW + x] = c;
	}

	// draw snakes (head = #, rest = +)
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

Vector Field::getSize(void) const
{
	return Vector(m_width, m_height);
}
