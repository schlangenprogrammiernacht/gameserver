#include <arpa/inet.h>

#include "Bot.h"
#include "Food.h"

#include "config.h"

#include "MsgPackUpdateTracker.h"

/* Private methods */

void MsgPackUpdateTracker::appendMessage(const msgpack::sbuffer &buf)
{
	uint32_t length = htonl(static_cast<uint32_t>(buf.size()));

	m_stream.write(reinterpret_cast<char*>(&length), sizeof(length));
	m_stream.write(buf.data(), buf.size());
}

/* Public methods */

MsgPackUpdateTracker::MsgPackUpdateTracker()
{
	reset();
}

void MsgPackUpdateTracker::foodConsumed(const Food &food,
		const std::shared_ptr<Bot> &by_bot)
{
	m_foodConsumeMessage->items.push_back({food.getGUID(), by_bot->getGUID()});
}

void MsgPackUpdateTracker::foodDecayed(const Food &food)
{
	m_foodDecayMessage->food_ids.push_back(food.getGUID());
}

void MsgPackUpdateTracker::foodSpawned(const Food &food)
{
	m_foodSpawnMessage->new_food.push_back(food);
}

void MsgPackUpdateTracker::botSpawned(const std::shared_ptr<Bot> &bot)
{
	MsgPackProtocol::BotSpawnMessage msg;
	msg.bot = bot;

	msgpack::sbuffer buf;
	msgpack::pack(buf, msg);
	appendMessage(buf);
}

void MsgPackUpdateTracker::botKilled(
		const std::shared_ptr<Bot> &killer,
		const std::shared_ptr<Bot> &victim)
{
	MsgPackProtocol::BotKillMessage msg;
	msg.killer_id = killer->getGUID();
	msg.victim_id = victim->getGUID();

	msgpack::sbuffer buf;
	msgpack::pack(buf, msg);
	appendMessage(buf);

	if (msg.killer_id == msg.victim_id)
	{
		botLogMessage(victim->getViewerKey(), std::string("reset."));
	}
	else
	{
		botLogMessage(killer->getViewerKey(), std::string("you killed ") + victim->getName() + std::string("."));
		botLogMessage(victim->getViewerKey(), std::string("you were killed by ") + killer->getName() + std::string("."));
	}
}

void MsgPackUpdateTracker::botMoved(const std::shared_ptr<Bot> &bot, std::size_t steps)
{
	// Fill BotMoveMessage
	MsgPackProtocol::BotMoveItem item;

	const Snake::SegmentList &segments = bot->getSnake()->getSegments();

	item.bot_id = bot->getGUID();
	item.new_segments.assign(segments.begin(), segments.begin() + steps);
	item.current_segment_radius = bot->getSnake()->getSegmentRadius();
	item.current_length = segments.size();

	m_botMoveMessage->items.push_back(item);

	// Fill BotMoveHeadMessage
	MsgPackProtocol::BotMoveHeadItem headItem;

	headItem.bot_id = bot->getGUID();
	headItem.mass = bot->getSnake()->getMass();
	headItem.new_head_positions = bot->getSnake()->getHeadPositionsDuringLastMove();

	m_botMoveHeadMessage->items.push_back(headItem);
}

void MsgPackUpdateTracker::botLogMessage(uint64_t viewerKey, const std::string& message)
{
	m_botLogMessage->items.push_back({viewerKey, message});
}

void MsgPackUpdateTracker::gameInfo(void)
{
	MsgPackProtocol::GameInfoMessage msg;

	msg.world_size_x = config::FIELD_SIZE_X;
	msg.world_size_y = config::FIELD_SIZE_Y;
	msg.food_decay_per_frame = config::FOOD_DECAY_STEP;

	msgpack::sbuffer buf;
	msgpack::pack(buf, msg);
	appendMessage(buf);
}

void MsgPackUpdateTracker::worldState(Field& field)
{
	MsgPackProtocol::WorldUpdateMessage msg;

	msg.bots = field.getBots();

	msg.food.reserve(1024);
	for (auto& food: field.getFoodMap()) // TODO directly serialize FoodMap
	{
		msg.food.push_back(food);
	}

	msgpack::sbuffer buf;
	msgpack::pack(buf, msg);
	appendMessage(buf);
}

void MsgPackUpdateTracker::tick(uint64_t frame_id)
{
	MsgPackProtocol::TickMessage msg;
	msg.frame_id = frame_id;
	msgpack::sbuffer buf;
	msgpack::pack(buf, msg);
	appendMessage(buf);
}

void MsgPackUpdateTracker::botStats(const std::shared_ptr<Bot> &bot)
{
	MsgPackProtocol::BotStatsItem item;

	item.bot_id = bot->getGUID();
	item.natural_food_consumed = bot->getConsumedNaturalFood();
	item.carrison_food_consumed = bot->getConsumedFoodHuntedByOthers();
	item.hunted_food_consumed = bot->getConsumedFoodHuntedBySelf();

	m_botStatsMessage->items.push_back(item);
}

std::string MsgPackUpdateTracker::serialize(void)
{
	// decayed food
	if(!m_foodDecayMessage->food_ids.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_foodDecayMessage);
		appendMessage(buf);
	}

	// spawned food
	if(!m_foodSpawnMessage->new_food.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_foodSpawnMessage);
		appendMessage(buf);
	}

	// consumed food
	if(!m_foodConsumeMessage->items.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_foodConsumeMessage);
		appendMessage(buf);
	}

	// moved bots
	if(!m_botMoveMessage->items.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_botMoveMessage);
		appendMessage(buf);
	}

	// moved bots (compressed version)
	if(!m_botMoveHeadMessage->items.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_botMoveHeadMessage);
		appendMessage(buf);
	}

	// bot statistics
	if(!m_botStatsMessage->items.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_botStatsMessage);
		appendMessage(buf);
	}

	// log messages
	if (!m_botLogMessage->items.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_botLogMessage);
		appendMessage(buf);
	}

	std::string result = m_stream.str();
	reset();
	return result;
}

void MsgPackUpdateTracker::reset(void)
{
	m_foodConsumeMessage = std::make_unique<MsgPackProtocol::FoodConsumeMessage>();
	m_foodSpawnMessage = std::make_unique<MsgPackProtocol::FoodSpawnMessage>();
	m_foodDecayMessage = std::make_unique<MsgPackProtocol::FoodDecayMessage>();
	m_botMoveMessage = std::make_unique<MsgPackProtocol::BotMoveMessage>();
	m_botMoveHeadMessage = std::make_unique<MsgPackProtocol::BotMoveHeadMessage>();
	m_botStatsMessage = std::make_unique<MsgPackProtocol::BotStatsMessage>();
	m_botLogMessage = std::make_unique<MsgPackProtocol::BotLogMessage>();

	m_stream.str("");
}

