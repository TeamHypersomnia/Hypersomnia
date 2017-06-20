#pragma once
#include "augs/graphics/rgba.h"

namespace augs {
	auto to_lua_value(const rgba r) {
		return r.stream_to(std::ostringstream()).str();
	}

	template <class I>
	void from_lua_value(I& in, rgba& r) {
		r.from_stream(std::istringstream(in.get<std::string>()));
	}
}