#pragma once

#include <msgpack.hpp>

#include "Field.h"
#include "Bot.h"

/*!
 * \file
 * \brief MessagePack adaptors for game objects.
 */

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

	template <> struct pack< std::shared_ptr<Snake::Segment> >
	{
		template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, std::shared_ptr<Snake::Segment> const& v) const
		{
			o.pack(v->pos.x());
			o.pack(v->pos.y());

			return o;
		}
	};

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
			o.pack(v->getSnake()->getSegments());

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
