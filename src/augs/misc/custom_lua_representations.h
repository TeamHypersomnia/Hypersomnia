#pragma once
#include "augs/graphics/rgba.h"

namespace augs {
	inline auto to_lua_value(const rgba r) {
		return r.stream_to(std::ostringstream()).str();
	}

	template <class I>
	void from_lua_value(I& in, rgba& r) {
		r.from_stream(std::istringstream(in.as<std::string>()));
	}

	inline auto to_lua_value(const ImVec4& r) {
		return to_lua_value(rgba(r));
	}

	template <class I>
	void from_lua_value(I& in, ImVec4& v) {
		rgba r;
		r.from_stream(std::istringstream(in.as<std::string>()));
		v = r;
	}
}