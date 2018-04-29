#include <iostream>
#include <vector>
#include <algorithm>

#include "Field.h"

Field::Field(real_t w, real_t h, std::size_t food_parts, std::unique_ptr<UpdateTracker> update_tracker)
	: m_width(w)
	, m_height(h)
	, m_updateTracker(std::move(update_tracker))
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

		Food food {true, Vector2D(x,y), value};
		m_updateTracker->foodSpawned(food);
		m_foodMap.addElement(food);
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

std::shared_ptr<Bot> Field::newBot(std::unique_ptr<db::BotScript> data, std::string& initErrorMessage)
{
	real_t x = (*m_positionXDistribution)(*m_rndGen);
	real_t y = (*m_positionYDistribution)(*m_rndGen);
	real_t heading = (*m_angleDegreesDistribution)(*m_rndGen);

	std::shared_ptr<Bot> bot = std::make_shared<Bot>(
		this,
		getCurrentFrame(),
		std::move(data),
		Vector2D(x,y),
		heading
	);

	initErrorMessage = "";
	if (bot->init(initErrorMessage))
	{
		std::cerr << "Created Bot with ID " << bot->getGUID() << std::endl;
		m_updateTracker->botSpawned(bot);
		m_bots.insert(bot);
	}
	else
	{
		m_updateTracker->botLogMessage(bot->getViewerKey(), "cannot start bot: " + initErrorMessage);
	}
	return bot;
}

void Field::decayFood(void)
{
	for (Food &item: m_foodMap)
	{
		if (item.decay()) {
			m_updateTracker->foodDecayed(item);
			if (item.shallRegenerate())
			{
				createStaticFood(1);
			}
		}
	};
}

void Field::removeFood()
{
	m_foodMap.erase_if([](const Food& item) {
		return item.shallBeRemoved();
	});
}

void Field::consumeFood(void)
{
	size_t newStaticFood = 0;
	for (auto &b: m_bots) {
		auto headPos = b->getSnake()->getHeadPosition();
		auto radius = b->getSnake()->getSegmentRadius() * config::SNAKE_CONSUME_RANGE;

		for (auto& fi: m_foodMap.getRegion(headPos, radius))
		{
			if (b->getSnake()->tryConsume(fi))
			{
				m_updateTracker->foodConsumed(fi, b);
				fi.markForRemove();
				if (fi.shallRegenerate())
				{
					newStaticFood++;
				}
			}
		}
	}
	createStaticFood(newStaticFood);
	updateMaxSegmentRadius();
}

void Field::moveAllBots(void)
{
	for(auto &b : m_bots) {
		std::unique_ptr<BotThreadPool::Job> job(new BotThreadPool::Job{b, 0});
		m_threadPool.addJob(std::move(job));
	}

	m_threadPool.waitForCompletion();

	// collision check for all bots
	std::unique_ptr<BotThreadPool::Job> job;
	while((job = m_threadPool.getProcessedJob()) != NULL) {
		std::shared_ptr<Bot> victim = job->bot;
		std::size_t steps = job->steps;

		std::shared_ptr<Bot> killer = victim->checkCollision();

		if (killer)
		{
			// collision detected, convert the colliding bot to food
			killBot(victim, killer);
		} else {
			// no collision, bot still alive
			m_updateTracker->botMoved(victim, steps);
		}
	}

	updateSnakeSegmentMap();
}

void Field::processLog()
{
	for (auto &b : m_bots)
	{
		for (auto &msg: b->getLogMessages())
		{
			m_updateTracker->botLogMessage(b->getViewerKey(), msg);
		}
		b->clearLogMessages();
		b->increaseLogCredit();
	}
}

void Field::tick()
{
	m_currentFrame++;
	m_updateTracker->tick(m_currentFrame);
}

const Field::BotSet& Field::getBots(void) const
{
	return m_bots;
}

std::shared_ptr<Bot> Field::getBotByDatabaseId(int id)
{
	for (auto& bot: m_bots)
	{
		if (bot->getDatabaseId() == id)
		{
			return bot;
		}
	}
	return nullptr;
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

		Food food {false, pos, value};
		m_updateTracker->foodSpawned(food);
		m_foodMap.addElement(food);
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

	// draw snakes (head = #, rest = +)
	for(auto &b: m_bots) {
		std::shared_ptr<Snake> snake = b->getSnake();

		bool first = true;
		for(auto &seg: snake->getSegments()) {
			size_t x = static_cast<size_t>(seg.pos().x());
			size_t y = static_cast<size_t>(seg.pos().y());

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

void Field::addBotKilledCallback(Field::BotKilledCallback callback)
{
	m_botKilledCallbacks.push_back(callback);
}

void Field::killBot(std::shared_ptr<Bot> victim, std::shared_ptr<Bot> killer)
{
	victim->getSnake()->convertToFood();
	m_bots.erase(victim);
	m_updateTracker->botKilled(killer, victim);

	// bot will eventually be recreated in callbacks
	for (auto& callback: m_botKilledCallbacks)
	{
		callback(victim, killer);
	}
}
