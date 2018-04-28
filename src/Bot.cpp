#include "Bot.h"

#include "Field.h"
#include "LuaBot.h"

Bot::Bot(Field *field, std::unique_ptr<LuaBot> luaBot, uint32_t startFrame, int databaseId, int databaseVersionId, const std::string &name, const Vector2D &startPos, real_t startHeading)
	: m_startFrame(startFrame)
	, m_databaseId(databaseId)
	, m_databaseVersionId(databaseVersionId)
	, m_name(name)
	, m_field(field)
	, m_lua_bot(std::move(luaBot))
	, m_moveCounter(0)
{
	// TODO: random start coordinates
	m_snake = std::make_shared<Snake>(field, startPos, 5, startHeading);
	m_heading = rand() * 360.0f / RAND_MAX;
}

Bot::~Bot()
{
}

std::size_t Bot::move(void)
{
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
	real_t maxCollisionDistance =
		m_snake->getSegmentRadius() + m_field->getMaxSegmentRadius();

	Vector2D headPos = m_snake->getHeadPosition();

	std::shared_ptr<Bot> retval = nullptr;
	for (auto &fi: m_field->getSegmentInfoMap().getRegion(headPos, maxCollisionDistance))
	{
		if(fi.bot->getGUID() == this->getGUID())
		{
			// prevent self-collision
			continue;
		}

		// get actual distance to segment
		real_t dist = (headPos - fi.pos()).squaredNorm();

		// get maximum distance for collision detection
		real_t collisionDist =
			m_snake->getSegmentRadius() + fi.bot->getSnake()->getSegmentRadius();
		collisionDist *= collisionDist; // square it

		if(dist < collisionDist) {
			// collision detected!
			retval = fi.bot;
			break;
		}
	}

	return retval;
}
