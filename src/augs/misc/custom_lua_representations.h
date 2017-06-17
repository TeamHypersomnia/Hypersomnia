#pragma once
#include "augs/graphics/rgba.h"

namespace augs {
	template <class O>
	void to_lua_value(const rgba r, O& out) {
		out = r.stream_to(std::ostringstream()).str();
	}

	template <class I>
	void from_lua_value(rgba& r, I& in) {
		r.from_stream(std::istringstream(in.get<std::string>()));
	}
}