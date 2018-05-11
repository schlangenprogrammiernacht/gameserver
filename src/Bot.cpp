/*
 * Schlangenprogrammiernacht: A programming game for GPN18.
 * Copyright (C) 2018  bytewerk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>

#include "Bot.h"

#include "Field.h"
#include "lua/LuaBot.h"

Bot::Bot(Field *field, uint32_t startFrame, std::unique_ptr<db::BotScript> dbData, const Vector2D &startPos, real_t startHeading)
	: m_field(field)
	, m_startFrame(startFrame)
	, m_dbData(std::move(dbData))
{
	m_snake = std::make_shared<Snake>(field, startPos, 5, startHeading);
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
	real_t directionChange;
	if (!m_lua_bot->step(directionChange, boost))
	{
		boost = false;
		directionChange = 0;
	}
	return m_snake->move(directionChange, boost);
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

uint32_t Bot::getFace()
{
	return m_lua_bot->getFace();
}

uint32_t Bot::getDogTag()
{
	return m_lua_bot->getDogTag();
}
