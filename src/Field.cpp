#include <iostream>
#include <vector>
#include <algorithm>

#include "LocalView.h"

#include "config.h"

#include "Field.h"

Field::Field(float_t w, float_t h, std::size_t food_parts,
				const std::shared_ptr<UpdateTracker> &update_tracker)
	: m_width(w), m_height(h), m_updateTracker(update_tracker),
	  m_maxSegmentRadius(0)
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

	m_angleDegreesDistribution =
		std::make_unique< std::uniform_real_distribution<float_t> >(-180, 180);
	m_angleRadDistribution =
		std::make_unique< std::uniform_real_distribution<float_t> >(-M_PI, M_PI);

	m_simple0To1Distribution =
		std::make_unique< std::uniform_real_distribution<float_t> >(0, 1);
}

void Field::updateGlobalView(void)
{
	m_globalView.rebuild(this);
}

void Field::updateMaxSegmentRadius(void)
{
	m_maxSegmentRadius = 0;

	for(auto &b: m_bots) {
		float_t segmentRadius = b->getSnake()->getSegmentRadius();

		if(segmentRadius > m_maxSegmentRadius) {
			m_maxSegmentRadius = segmentRadius;
		}
	}
}

void Field::newBot(const std::string &name)
{
	float_t x = (*m_positionXDistribution)(*m_rndGen);
	float_t y = (*m_positionYDistribution)(*m_rndGen);
	float_t heading = (*m_angleDegreesDistribution)(*m_rndGen);

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
			dfi = m_dynamicFood.erase(dfi);
		} else {
			dfi++;
		}
	}
}

void Field::consumeFood(void)
{
	// update the global view for faster processing
	updateGlobalView();

	for(auto &b: m_bots) {
		std::size_t foodToGenerate = 0;

		// create a LocalView for this bot which contains only the food close to
		// the bot’s head
		std::shared_ptr<LocalView> localView = m_globalView.extractLocalView(
				b->getSnake()->getHeadPosition(),
				b->getSnake()->getSegmentRadius() * config::SNAKE_CONSUME_RANGE);

		const LocalView::FoodInfoList &foodList = localView->getFood();

		// iterate over all food items, check if they can be consumed and if so,
		// consume it and regenerate it if it was a static food item
		auto fi = foodList.begin();
		while(fi != foodList.end()) {
			if(b->getSnake()->canConsume(fi->food)) {
				b->getSnake()->consume(fi->food);
				m_updateTracker->foodConsumed(fi->food, b);

				if(m_staticFood.count(fi->food) > 0) {
					m_staticFood.erase(fi->food);
					foodToGenerate++;
				} else if(m_dynamicFood.count(fi->food) > 0) {
					m_dynamicFood.erase(fi->food);
				}
			}

			fi++;
		}

		createStaticFood(foodToGenerate);
	}

	updateMaxSegmentRadius();
}

void Field::moveAllBots(void)
{
	auto bi = m_bots.begin();
	while(bi != m_bots.end()) {
		size_t steps = (*bi)->move();

		std::shared_ptr<Bot> killer = (*bi)->checkCollision();

		if(killer) {
			// collision detected, convert the colliding bot to food
			(*bi)->getSnake()->convertToFood();

			// keep a pointer which can be handed to the update tracker
			std::shared_ptr<Bot> victim = *bi;

			// remove the bot from the field
			bi = m_bots.erase(bi);

			newBot(victim->getName());

			m_updateTracker->botKilled(killer, victim);
		} else {
			// no collision, bot still alive
			m_updateTracker->botMoved((*bi), steps);

			bi++;
		}
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

void Field::createDynamicFood(float_t totalValue, const Vector &center, float_t radius)
{
	// create at least 1 food item
	std::size_t count = 1 + totalValue / config::FOOD_SIZE_MEAN;

	for(std::size_t i = 0; i < count; i++) {
		float_t value = (*m_foodSizeDistribution)(*m_rndGen);
		float_t x     = (*m_positionXDistribution)(*m_rndGen);
		float_t y     = (*m_positionYDistribution)(*m_rndGen);

		Vector offset(radius * (*m_simple0To1Distribution)(*m_rndGen), 0);
		offset.rotate((*m_angleRadDistribution)(*m_rndGen));

		Vector pos = center + offset;

		std::shared_ptr<Food> newFood =
			std::make_shared<Food>(this, pos, value);

		m_updateTracker->foodSpawned(newFood);
		m_dynamicFood.insert( newFood );
	}
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

const GlobalView& Field::getGlobalView(void) const
{
	return m_globalView;
}

float_t Field::getMaxSegmentRadius(void) const
{
	return m_maxSegmentRadius;
}
