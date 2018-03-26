#include "Bot.h"
#include "Field.h"

Bot::Bot(Field *field, const std::string &name, const Vector2D &startPos, float_t startHeading)
	: m_name(name), m_field(field), m_moveCounter(0)
{
	// TODO: random start coordinates
	m_snake = std::make_shared<Snake>(field, startPos, 5, startHeading);

	m_heading = rand() * 360.0 / RAND_MAX;
}

std::size_t Bot::move(void)
{
	if((++m_moveCounter % 200) == 0) {
		m_heading = rand() * 360.0 / RAND_MAX;
	}

	return m_snake->move(m_heading); // direction in degrees
}

std::shared_ptr<Bot> Bot::checkCollision(void) const
{
	float_t maxCollisionDistance =
		m_snake->getSegmentRadius() + m_field->getMaxSegmentRadius();

	Vector2D headPos = m_snake->getHeadPosition();

	std::shared_ptr<Bot> retval = nullptr;
	m_field->getSegmentInfoMap().processElements(
		headPos,
		maxCollisionDistance,
		[this, &headPos, &retval](const Field::SnakeSegmentInfo &fi)
		{
			if(fi.bot->getGUID() == this->getGUID())
			{
				// prevent self-collision
				return true;
			}

			// get actual distance to segment
			float_t dist = (headPos - fi.pos()).squaredNorm();

			// get maximum distance for collision detection
			float_t collisionDist =
				m_snake->getSegmentRadius() + fi.bot->getSnake()->getSegmentRadius();
			collisionDist *= collisionDist; // square it

			if(dist < collisionDist) {
				// collision detected!
				retval = fi.bot;
				return false;
			}

			return true;
		}
	);

	return retval;
}

std::shared_ptr<Snake> Bot::getSnake(void) const
{
	return m_snake;
}

const std::string& Bot::getName(void) const
{
	return m_name;
}
