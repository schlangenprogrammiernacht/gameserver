#include <iostream>

#include "Bot.h"

#include "Field.h"
#include "LuaBot.h"

Bot::Bot(Field *field, uint32_t startFrame, std::unique_ptr<db::BotScript> dbData, const Vector2D &startPos, real_t startHeading)
	: m_field(field)
	, m_startFrame(startFrame)
	, m_dbData(std::move(dbData))
{
	m_snake = std::make_shared<Snake>(field, startPos, 5, startHeading);
	m_heading = rand() * 360.0f / RAND_MAX;
	m_lua_bot = std::make_unique<LuaBot>(*this, m_dbData->code);
}

Bot::~Bot()
{
	std::cerr << "Bot consume stats: " <<
		m_consumedNaturalFood << " / " <<
		m_consumedFoodHuntedByOthers << " / " <<
		m_consumedFoodHuntedBySelf << std::endl;
}

bool Bot::init(std::string& initErrorMessage)
{
	return m_lua_bot->init(initErrorMessage);
}

std::size_t Bot::move(void)
{
	bool boost;
	float new_heading;
	if (m_lua_bot->step(new_heading, boost))
	{
		new_heading = fmod(new_heading, 360);
		if (new_heading<0)
		{
			new_heading += 360;
		}
		m_heading = new_heading;
	}

	return m_snake->move(m_heading, boost); // direction in degrees
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

void Bot::updateConsumeStats(const Food &food)
{
	std::shared_ptr<Bot> hunter = food.getHunter();

	if(!hunter) {
		// nullptr indicates natural food
		m_consumedNaturalFood += food.getValue();
	} else if(this->getGUID() == hunter->getGUID()) {
		// food was hunted by this bot
		m_consumedFoodHuntedBySelf += food.getValue();
	} else {
		// food was hunted by another bot
		m_consumedFoodHuntedByOthers += food.getValue();
	}
}

void Bot::increaseLogCredit()
{
	m_logCredit = std::min(
		m_logCredit+config::LOG_CREDITS_PER_FRAME,
		config::LOG_MAX_CREDITS
	);
}

bool Bot::appendLogMessage(const std::string& data, bool checkCredit)
{
	if (checkCredit)
	{
		if (m_logCredit<1) { return false; }
		m_logCredit -= 1;
	}
	m_logMessages.push_back(data.substr(0, config::LOG_MAX_MESSAGE_SIZE));
	return true;
}

std::vector<uint32_t> Bot::getColors()
{
	return m_lua_bot->getColors();
}

real_t Bot::getSightRadius() const
{
	return 50.0f + 15.0f * getSnake()->getSegmentRadius();
}
