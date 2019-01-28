#pragma once
#include "augs/readwrite/byte_readwrite_traits.h"

namespace net_messages {
	template <class Stream, class V>
	bool serialize_trivial_as_bytes(Stream& s, V& v) {
		static_assert(augs::is_byte_readwrite_appropriate_v<augs::memory_stream, V>);
		serialize_bytes(s, (uint8_t*)&v, sizeof(V));
		return true;
	}

	template <class Stream, class E>
	bool serialize_enum(Stream& s, E& e) {
		auto ee = static_cast<int>(e);
		serialize_int(s, ee, 0, static_cast<int>(E::COUNT));
		e = static_cast<E>(ee);
		return true;
	}
}
