#include <iostream>
#include <vector>
#include <algorithm>

#include "Field.h"

Field::Field(real_t w, real_t h, std::size_t food_parts, const std::shared_ptr<UpdateTracker> &update_tracker)
	: m_width(w)
	, m_height(h)
	, m_updateTracker(update_tracker)
	, m_foodMap(static_cast<size_t>(w), static_cast<size_t>(h), config::SPATIAL_MAP_RESERVE_COUNT)
	, m_segmentInfoMap(static_cast<size_t>(w), static_cast<size_t>(h), config::SPATIAL_MAP_RESERVE_COUNT)
	, m_threadPool(std::thread::hardware_concurrency())
{
	setupRandomness();
	createStaticFood(food_parts);
}

void Field::createStaticFood(std::size_t count)
{
	for(std::size_t i = 0; i < count; i++) {
		real_t value = (*m_foodSizeDistribution)(*m_rndGen);
		real_t x     = (*m_positionXDistribution)(*m_rndGen);
		real_t y     = (*m_positionYDistribution)(*m_rndGen);

		std::shared_ptr<Food> newFood =
			std::make_shared<Food>(this, Vector2D(x, y), value);

		m_updateTracker->foodSpawned(newFood);
		m_staticFood.insert( newFood );
	}
}

void Field::setupRandomness(void)
{
	std::random_device rd;
	m_rndGen = std::make_unique<std::mt19937>(rd());

	m_foodSizeDistribution = std::make_unique< std::normal_distribution<real_t> >(
			config::FOOD_SIZE_MEAN, config::FOOD_SIZE_STDDEV);

	m_positionXDistribution =
		std::make_unique< std::uniform_real_distribution<real_t> >(0, m_width);
	m_positionYDistribution =
		std::make_unique< std::uniform_real_distribution<real_t> >(0, m_height);

	m_angleDegreesDistribution =
		std::make_unique< std::uniform_real_distribution<real_t> >(-180, 180);
	m_angleRadDistribution =
		std::make_unique< std::uniform_real_distribution<real_t> >(-M_PI, M_PI);

	m_simple0To1Distribution =
		std::make_unique< std::uniform_real_distribution<real_t> >(0, 1);
}

void Field::updateFoodMap()
{
	m_foodMap.clear();
	for (auto &f : m_staticFood)
	{
		m_foodMap.addElement(f);
	}
	for (auto &f : m_dynamicFood)
	{
		m_foodMap.addElement(f);
	}
}

void Field::updateSnakeSegmentMap()
{
	m_segmentInfoMap.clear();
	for (auto &b : m_bots)
	{
		for(auto &s : b->getSnake()->getSegments())
		{
			m_segmentInfoMap.addElement({s, b});
		}
	}
}

void Field::updateMaxSegmentRadius(void)
{
	m_maxSegmentRadius = 0;

	for(auto &b: m_bots) {
		real_t segmentRadius = b->getSnake()->getSegmentRadius();

		if(segmentRadius > m_maxSegmentRadius) {
			m_maxSegmentRadius = segmentRadius;
		}
	}
}

void Field::newBot(const std::string &name)
{
	real_t x = (*m_positionXDistribution)(*m_rndGen);
	real_t y = (*m_positionYDistribution)(*m_rndGen);
	real_t heading = (*m_angleDegreesDistribution)(*m_rndGen);

	std::shared_ptr<Bot> bot = std::make_shared<Bot>(this, name, Vector2D(x,y), heading);

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

	// step 3: refresh food spatial map for faster access
	updateFoodMap();
}

void Field::consumeFood(void)
{
	for (auto &b: m_bots) {
		std::size_t foodToGenerate = 0;

		m_foodMap.processElements(
			b->getSnake()->getHeadPosition(),
			b->getSnake()->getSegmentRadius() * config::SNAKE_CONSUME_RANGE,
			[this, &b, &foodToGenerate](const FoodInfo& fi)
			{
				if (!b->getSnake()->canConsume(fi.food)) { return true; }

				b->getSnake()->consume(fi.food);
				m_updateTracker->foodConsumed(fi.food, b);

				if (m_staticFood.count(fi.food) > 0)
				{
					m_staticFood.erase(fi.food);
					foodToGenerate++;
				}
				else if (m_dynamicFood.count(fi.food) > 0)
				{
					m_dynamicFood.erase(fi.food);
				}

				return true;
			}
		);

		createStaticFood(foodToGenerate); // TODO should this be done outside the bot loop?
	}

	updateMaxSegmentRadius();
}

void Field::moveAllBots(void)
{
	for(auto &b : m_bots) {
		std::unique_ptr<BotThreadPool::Job> job(new BotThreadPool::Job{b, 0});
		m_threadPool.addJob(std::move(job));
	}

	m_threadPool.run();

	// collision check for all bots
	std::unique_ptr<BotThreadPool::Job> job;
	while(job = m_threadPool.getProcessedJob()) {
		std::shared_ptr<Bot> victim = job->bot;
		std::size_t steps = job->steps;

		std::shared_ptr<Bot> killer = victim->checkCollision();

		if(killer) {
			// collision detected, convert the colliding bot to food
			victim->getSnake()->convertToFood();

			// remove the bot from the field
			m_bots.erase(victim);

			newBot(victim->getName());

			m_updateTracker->botKilled(killer, victim);
		} else {
			// no collision, bot still alive
			m_updateTracker->botMoved(victim, steps);
		}
	}

	updateSnakeSegmentMap();
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

void Field::createDynamicFood(real_t totalValue, const Vector2D &center, real_t radius)
{
	// create at least 1 food item
	std::size_t count = 1 + totalValue / config::FOOD_SIZE_MEAN;

	for(std::size_t i = 0; i < count; i++) {
		real_t value = (*m_foodSizeDistribution)(*m_rndGen);

		real_t rndRadius = radius * (*m_simple0To1Distribution)(*m_rndGen);
		real_t rndAngle = (*m_angleRadDistribution)(*m_rndGen);

		Vector2D offset(cos(rndAngle), sin(rndAngle));
		offset *= rndRadius;

		Vector2D pos = wrapCoords(center + offset);

		std::shared_ptr<Food> newFood =
			std::make_shared<Food>(this, pos, value);

		m_updateTracker->foodSpawned(newFood);
		m_dynamicFood.insert( newFood );
	}
}

Vector2D Field::wrapCoords(const Vector2D &v) const
{
	real_t x = v.x();
	real_t y = v.y();

	while(x < 0) {
		x += m_width;
	}

	while(x > m_width) {
		x -= m_width;
	}

	while(y < 0) {
		y += m_height;
	}

	while(y > m_height) {
		y -= m_height;
	}

	return {x, y};
}

Vector2D Field::unwrapCoords(const Vector2D &v, const Vector2D &ref) const
{
	real_t x = v.x();
	real_t y = v.y();

	while((x - ref.x()) < -m_width/2) {
		x += m_width;
	}

	while((x - ref.x()) > m_width/2) {
		x -= m_width;
	}

	while((y - ref.y()) < -m_height/2) {
		y += m_height;
	}

	while((y - ref.y()) > m_height/2) {
		y -= m_height;
	}

	return {x, y};
}

Vector2D Field::unwrapRelativeCoords(const Vector2D& relativeCoords) const
{
	real_t x = fmod(relativeCoords.x(), m_width);
	real_t y = fmod(relativeCoords.y(), m_height);
	if (x > m_width/2) { x -= m_width; }
	if (x < (-(int)m_width/2)) { x += m_width; }
	if (y > m_height/2) { y -= m_height; }
	if (y < (-(int)m_height/2)) { y += m_height; }
	return Vector2D { x, y };
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
		const Vector2D &pos = f->pos();

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
			size_t x = static_cast<size_t>(seg->pos().x());
			size_t y = static_cast<size_t>(seg->pos().y());

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

Vector2D Field::getSize(void) const
{
	return Vector2D(m_width, m_height);
}

real_t Field::getMaxSegmentRadius(void) const
{
	return m_maxSegmentRadius;
}
