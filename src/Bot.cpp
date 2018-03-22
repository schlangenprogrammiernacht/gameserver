#include "LocalView.h"

#include "Field.h"

#include "Bot.h"
#include "LuaBot.h"

Bot::Bot(Field *field, const std::string &name, const Vector2D &startPos, float_t startHeading)
	: m_name(name), m_field(field), m_moveCounter(0)
{
	// TODO: random start coordinates
	m_snake = std::make_shared<Snake>(field, startPos, 5, startHeading);

	m_heading = rand() * 360.0f / RAND_MAX;
	m_lua_bot = std::make_unique<LuaBot>();
}

Bot::~Bot()
{
}

std::size_t Bot::move(void)
{
	auto localView = createLocalView(20*m_snake->getSegmentRadius());
	bool boost;
	float new_heading;
	if (m_lua_bot->step(*this, new_heading, boost))
	{
		new_heading = fmod(new_heading, 360);
		if (new_heading<0)
		{
			new_heading += 360;
		}
		m_heading = new_heading;
	}

	return m_snake->move(m_heading); // direction in degrees
}

std::shared_ptr<Bot> Bot::checkCollision(void) const
{
	const GlobalView &globalView = m_field->getGlobalView();

	float_t maxCollisionDistance =
		m_snake->getSegmentRadius() + m_field->getMaxSegmentRadius();

	Vector2D headPos = m_snake->getHeadPosition();

	// create a LocalView for this bot which contains only the snake segments
	// close to the bot’s head
	auto localView = globalView.extractLocalView(
			headPos,
			maxCollisionDistance);

	for(auto &fi: localView->getSnakeSegments()) {
		if(fi.bot->getGUID() == this->getGUID()) {
			// prevent self-collision
			continue;
		}

		// get actual distance to segment
		float_t dist = (headPos - fi.pos).squaredNorm();

		// get maximum distance for collision detection
		float_t collisionDist =
			m_snake->getSegmentRadius() + fi.bot->getSnake()->getSegmentRadius();
		collisionDist *= collisionDist; // square it

		if(dist < collisionDist) {
			// collision detected!
			return fi.bot;
		}
	}

	return NULL;
}

std::shared_ptr<Snake> Bot::getSnake(void) const
{
	return m_snake;
}

const std::string& Bot::getName(void) const
{
	return m_name;
}

std::shared_ptr<LocalView> Bot::createLocalView(float_t radius) const
{
	return getGlobalView().extractLocalView(m_snake->getHeadPosition(), radius);
}

const GlobalView &Bot::getGlobalView() const
{
	return m_field->getGlobalView();
}
