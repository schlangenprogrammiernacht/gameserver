#pragma once

#include <cstdint>
#include <vector>

#include <msgpack.hpp>

#include "types.h"

#include "Field.h"
#include "Bot.h"

namespace MsgPackProtocol
{
	enum
	{
		MESSAGE_TYPE_GAME_INFO = 0x00,
		MESSAGE_TYPE_WORLD_UPDATE = 0x01,

		MESSAGE_TYPE_TICK = 0x10,

		MESSAGE_TYPE_BOT_SPAWN = 0x20,
		MESSAGE_TYPE_BOT_KILL = 0x21,
		MESSAGE_TYPE_BOT_MOVE = 0x22,
		MESSAGE_TYPE_BOT_LOG = 0x23,
		MESSAGE_TYPE_BOT_STATS = 0x24,
		MESSAGE_TYPE_BOT_MOVE_HEAD = 0x25,

		MESSAGE_TYPE_FOOD_SPAWN = 0x30,
		MESSAGE_TYPE_FOOD_CONSUME = 0x31,
		MESSAGE_TYPE_FOOD_DECAY = 0x32,

		MESSAGE_TYPE_PLAYER_INFO = 0xF0,
	};

	static constexpr const uint8_t PROTOCOL_VERSION = 1;

	struct GameInfoMessage
	{
		double world_size_x = 0;
		double world_size_y = 0;
		double food_decay_per_frame = 0;

		double snake_distance_per_step = 0;
		double snake_segment_distance_factor = 0;
		double snake_segment_distance_exponent = 0;
		double snake_pull_factor = 0;
	};

	struct PlayerInfoMessage
	{
		guid_t player_id; // id der von dieser Verbindung gesteuerten Schlange
	};

	struct TickMessage
	{
		guid_t frame_id; // frame counter since start of server
	};

	struct WorldUpdateMessage
	{
		Field::BotSet bots;
		std::vector<Food> food;
	};

	struct BotSpawnMessage
	{
		std::shared_ptr<Bot> bot;
	};

	struct BotMoveItem
	{
		guid_t bot_id;
		std::vector< Snake::Segment > new_segments;
		uint32_t current_length;
		uint32_t current_segment_radius;
	};

	struct BotMoveMessage
	{
		std::vector<BotMoveItem> items;
	};

	struct BotMoveHeadItem
	{
		guid_t bot_id;
		double mass;

		// one head position for each step moved in this frame, in temporal order
		std::vector< Vector2D > new_head_positions;
	};

	struct BotMoveHeadMessage
	{
		std::vector<BotMoveHeadItem> items;
	};

	struct BotKillMessage
	{
		guid_t killer_id;
		guid_t victim_id; // victim is deleted in this frame
	};

	struct FoodSpawnMessage
	{
		std::vector<Food> new_food;
	};

	struct FoodConsumeItem
	{
		guid_t food_id; // food is deleted in this frame
		guid_t bot_id; // bot consuming the food
	};

	struct FoodConsumeMessage
	{
		std::vector<FoodConsumeItem> items;
	};

	struct FoodDecayMessage
	{
		std::vector<guid_t> food_ids; // food is deleted in this frame
	};

	struct BotStatsItem
	{
		guid_t bot_id;
		double natural_food_consumed;
		double carrison_food_consumed;
		double hunted_food_consumed;
		double mass;
	};

	struct BotStatsMessage
	{
		std::vector<BotStatsItem> items;
	};

	struct BotLogItem
	{
		uint64_t viewer_key;
		std::string message;
	};

	struct BotLogMessage
	{
		std::vector<BotLogItem> items;
	};
}

namespace msgpack {
	MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
		namespace adaptor {

			template <> struct pack<MsgPackProtocol::GameInfoMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::GameInfoMessage const& v) const
				{
					o.pack_array(9);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_GAME_INFO));
					o.pack(v.world_size_x);
					o.pack(v.world_size_y);
					o.pack(v.food_decay_per_frame);
					o.pack(v.snake_distance_per_step);
					o.pack(v.snake_segment_distance_factor);
					o.pack(v.snake_segment_distance_exponent);
					o.pack(v.snake_pull_factor);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::PlayerInfoMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::PlayerInfoMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_PLAYER_INFO));
					o.pack(v.player_id);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::TickMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::TickMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_TICK));
					o.pack(v.frame_id);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::WorldUpdateMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::WorldUpdateMessage const& v) const
				{
					o.pack_array(4);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_WORLD_UPDATE));
					o.pack(v.bots);
					o.pack(v.food);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotSpawnMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotSpawnMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_BOT_SPAWN));
					o.pack(v.bot);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotMoveMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotMoveMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_BOT_MOVE));
					o.pack(v.items);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotMoveItem>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotMoveItem const& v) const
				{
					o.pack_array(4);
					o.pack(v.bot_id);
					o.pack(v.new_segments);
					o.pack(v.current_length);
					o.pack(v.current_segment_radius);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotMoveHeadMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotMoveHeadMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_BOT_MOVE_HEAD));
					o.pack(v.items);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotMoveHeadItem>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotMoveHeadItem const& v) const
				{
					o.pack_array(3);
					o.pack(v.bot_id);
					o.pack(v.mass);
					o.pack(v.new_head_positions);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotKillMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotKillMessage const& v) const
				{
					o.pack_array(4);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_BOT_KILL));
					o.pack(v.killer_id);
					o.pack(v.victim_id);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::FoodSpawnMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::FoodSpawnMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_FOOD_SPAWN));
					o.pack(v.new_food);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::FoodConsumeMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::FoodConsumeMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_FOOD_CONSUME));
					o.pack(v.items);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::FoodConsumeItem>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::FoodConsumeItem const& v) const
				{
					o.pack_array(2);
					o.pack(v.food_id);
					o.pack(v.bot_id);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::FoodDecayMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::FoodDecayMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_FOOD_DECAY));
					o.pack(v.food_ids);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotStatsMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotStatsMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_BOT_STATS));
					o.pack(v.items);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotStatsItem>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotStatsItem const& v) const
				{
					o.pack_array(4);
					o.pack(v.bot_id);
					o.pack(v.natural_food_consumed);
					o.pack(v.carrison_food_consumed);
					o.pack(v.hunted_food_consumed);
					o.pack(v.mass);
					return o;
				}
			};

			template <> struct pack<Snake::Segment>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Snake::Segment const& v) const
				{
					o.pack_array(2);
					o.pack(v.pos().x());
					o.pack(v.pos().y());

					return o;
				}
			};

			template <> struct pack<Vector2D>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Vector2D const& v) const
				{
					o.pack_array(2);
					o.pack(v.x());
					o.pack(v.y());

					return o;
				}
			};

			template<>
				struct convert<Snake::Segment> {
					msgpack::object const& operator()(msgpack::object const& o, Snake::Segment& v) const {
						if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
						if (o.via.array.size != 2) throw msgpack::type_error();
						v = Snake::Segment{{
							o.via.array.ptr[0].as<float>(),
								o.via.array.ptr[1].as<float>()
						}};
						return o;
					}
				};

			template <> struct pack<Food>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Food const& v) const
				{
					o.pack_array(4);
					o.pack(v.getGUID());
					o.pack(v.pos().x());
					o.pack(v.pos().y());
					o.pack(v.getValue());
					return o;
				}
			};

			template <> struct pack< std::shared_ptr<Bot>>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, std::shared_ptr<Bot> const& v) const
				{
					o.pack_array(9);
					o.pack(v->getGUID());
					o.pack(v->getName());
					o.pack(v->getDatabaseVersionId());
					o.pack(v->getFace());
					o.pack(v->getDogTag());
					o.pack(v->getColors());
					o.pack(v->getSnake()->getMass());
					o.pack(v->getSnake()->getSegmentRadius());
					o.pack(v->getSnake()->getSegments());
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotLogMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotLogMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_BOT_LOG));
					o.pack(v.items);
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotLogItem>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotLogItem const& v) const
				{
					o.pack_array(2);
					o.pack(v.viewer_key);
					o.pack(v.message);
					return o;
				}
			};
		} // namespace adaptor
	} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
