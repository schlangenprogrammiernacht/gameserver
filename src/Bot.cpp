#include "LocalView.h"

#include "Field.h"

#include "Bot.h"
#include "LuaBot.h"

Bot::Bot(Field *field, const std::string &name, const Vector &startPos, float_t startHeading)
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
	bool speed;
	float new_heading;
	if (m_lua_bot->step(m_heading, new_heading, speed))
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

	Vector headPos = m_snake->getHeadPosition();

	// create a LocalView for this bot which contains only the snake segments
	// close to the bot’s head
	std::shared_ptr<LocalView> localView = globalView.extractLocalView(
			headPos,
			maxCollisionDistance);

	for(auto &fi: localView->getSnakeSegments()) {
		if(fi.bot->getGUID() == this->getGUID()) {
			// prevent self-collision
			continue;
		}

		// get actual distance to segment
		float_t dist = headPos.squareDistanceTo(fi.pos);

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
