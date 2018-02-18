#include <arpa/inet.h>

#include "Bot.h"
#include "Food.h"

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

// MessagePack adaptors

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

	template <> struct pack< std::shared_ptr<Food> >
	{
		template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, std::shared_ptr<Food> const& v) const
		{
			o.pack(v->getGUID());

			const Vector &pos = v->getPosition();
			o.pack(pos.x());
			o.pack(pos.y());

			o.pack(v->getValue());

			return o;
		}
	};

	template <> struct pack< std::shared_ptr<Bot> >
	{
		template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, std::shared_ptr<Bot> const& v) const
		{
			o.pack(v->getGUID());

			o.pack(std::string("unnamed")); // FIXME: pack bot name here

			o.pack(1.0); // FIXME: segment radius

			// segments
			const Snake::SegmentList &segments = v->getSnake()->getSegments();
			o.pack_array(segments.size());
			for(auto &s: segments) {
				o.pack(s->pos.x());
				o.pack(s->pos.y());
			}

			// FIXME: colormap: array of RGB values
			o.pack_array(3);
			o.pack(0xFF0000);
			o.pack(0x00FF00);
			o.pack(0x0000FF);

			return o;
		}
	};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
