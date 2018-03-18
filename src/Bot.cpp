#include "LocalView.h"

#include "Field.h"

#include "Bot.h"

Bot::Bot(Field *field, const std::string &name, const Vector &startPos, float_t startHeading)
	: m_name(name), m_field(field), m_moveCounter(0)
{
	// TODO: random start coordinates
	m_snake = std::make_shared<Snake>(field, startPos, 5, startHeading);

	m_heading = rand() * 360.0 / RAND_MAX;
	// TODO: move lua setup out of constructor
	m_lua_state.open_libraries();
	m_lua_state.script_file("lua/quota.lua");
	m_lua_safe_env = sol::environment(m_lua_state, sol::create);
	m_lua_safe_env["math"] = m_lua_state["math"];
	m_lua_state.safe_script_file("lua/demobot.lua", m_lua_safe_env);
}

std::size_t Bot::move(void)
{
	m_lua_state.script("set_time_quota(0.1)");
	m_lua_state.script("set_instruction_quota(1000000)");

	float_t lua_heading = m_heading;
	try
	{
		lua_heading = m_lua_safe_env["run"](m_heading);
		if (lua_heading<0) { lua_heading += 360; }
		if (lua_heading>=360) { lua_heading -= 360; }
		m_heading = lua_heading;
	}
	catch (const sol::error& e)
	{
		printf("script aborted: %s\n", e.what());
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
