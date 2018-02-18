#include <arpa/inet.h>

#include "Bot.h"
#include "Food.h"

#include "config.h"

#include "MsgPackAdaptors.h"

#include "MsgPackUpdateTracker.h"

/* Private methods */

void MsgPackUpdateTracker::appendPacket(
		MsgPackUpdateTracker::MessageType type,
		uint8_t protocolVersion,
		const std::string &data)
{
	// build the full packet (including version and type)
	std::ostringstream packetBuilder;

	msgpack::pack(packetBuilder, protocolVersion);
	msgpack::pack(packetBuilder, static_cast<int>(type));

	packetBuilder.write(data.data(), data.size());

	std::string packet = packetBuilder.str();

	// write the full packet to the packet stream
	std::uint32_t length = ntohl(static_cast<uint32_t>(packet.size()));

	m_stream.write(reinterpret_cast<const char*>(&length), sizeof(length));
	m_stream.write(packet.data(), packet.size());
}

/* Public methods */

MsgPackUpdateTracker::MsgPackUpdateTracker()
{
	reset();
}

void MsgPackUpdateTracker::foodConsumed(
		const std::shared_ptr<Food> &food,
		const std::shared_ptr<Bot> &by_bot)
{
	FoodConsumedItem item = {
		.botID = by_bot->getGUID(),
		.foodID = by_bot->getGUID()
	};

	m_consumedFood.push_back(item);
}

void MsgPackUpdateTracker::foodDecayed(const std::shared_ptr<Food> &food)
{
	m_decayedFood.push_back(food->getGUID());
}

void MsgPackUpdateTracker::foodSpawned(const std::shared_ptr<Food> &food)
{
	m_spawnedFood.push_back(food);
}

void MsgPackUpdateTracker::botSpawned(const std::shared_ptr<Bot> &bot)
{
	std::ostringstream tmpStream;
	msgpack::pack(tmpStream, bot);
	appendPacket(BotSpawn, 1, tmpStream.str());
}

void MsgPackUpdateTracker::botKilled(
		const std::shared_ptr<Bot> &killer,
		const std::shared_ptr<Bot> &victim)
{
	std::ostringstream tmpStream;
	msgpack::pack(tmpStream, killer->getGUID());
	msgpack::pack(tmpStream, victim->getGUID());
	appendPacket(BotKill, 1, tmpStream.str());
}

void MsgPackUpdateTracker::gameInfo(void)
{
	std::ostringstream tmpStream;

	msgpack::pack(tmpStream, config::FIELD_SIZE_X);
	msgpack::pack(tmpStream, config::FIELD_SIZE_Y);
	msgpack::pack(tmpStream, config::FOOD_DECAY_STEP);

	appendPacket(GameInfo, 1, tmpStream.str());
}

void MsgPackUpdateTracker::worldState(const std::shared_ptr<Field> &field)
{
	std::ostringstream tmpStream;

	// add the bots
	msgpack::pack(tmpStream, field->getBots());

	// add all the food
	const Field::FoodSet &staticFood = field->getStaticFood();
	const Field::FoodSet &dynamicFood = field->getDynamicFood();

	Field::FoodSet allFood;
	allFood.insert(staticFood.begin(), staticFood.end());
	allFood.insert(dynamicFood.begin(), dynamicFood.end());

	msgpack::pack(tmpStream, allFood);

	appendPacket(WorldUpdate, 1, tmpStream.str());
}

std::string MsgPackUpdateTracker::serialize(void)
{
	std::ostringstream tmpStream;

	// decayed food
	if(!m_decayedFood.empty()) {
		tmpStream.str("");
		msgpack::pack(tmpStream, m_decayedFood);
		appendPacket(FoodDecay, 1, tmpStream.str());
	}

	// spawned food
	if(!m_spawnedFood.empty()) {
		tmpStream.str("");
		msgpack::pack(tmpStream, m_spawnedFood);
		appendPacket(FoodSpawn, 1, tmpStream.str());
	}

	// consumed food
	if(!m_consumedFood.empty()) {
		tmpStream.str("");
		msgpack::pack(tmpStream, m_consumedFood);
		appendPacket(FoodConsume, 1, tmpStream.str());
	}

	std::string result = m_stream.str();
	reset();
	return result;
}

void MsgPackUpdateTracker::reset(void)
{
	m_decayedFood.clear();
	m_consumedFood.clear();
	m_spawnedFood.clear();

	m_stream.str("");
}

