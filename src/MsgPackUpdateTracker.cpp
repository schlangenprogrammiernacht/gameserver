#include "MsgPackUpdateTracker.h"

MsgPackUpdateTracker::MsgPackUpdateTracker()
{
	reset();
}

void MsgPackUpdateTracker::foodConsumed(
		const std::shared_ptr<Food> &food,
		const std::shared_ptr<Bot> &by_bot)
{
	m_stream << "food consumed\n";
}

void MsgPackUpdateTracker::foodDecayed(const std::shared_ptr<Food> &food)
{
	m_stream << "food decayed\n";
}

void MsgPackUpdateTracker::foodSpawned(const std::shared_ptr<Food> &food)
{
	m_stream << "food spawned\n";
}

std::string MsgPackUpdateTracker::getSerializedEvents(void)
{
	return m_stream.str();
}

void MsgPackUpdateTracker::reset(void)
{
	m_stream.str("");
}
