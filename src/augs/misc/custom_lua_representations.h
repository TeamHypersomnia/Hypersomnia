#pragma once
#include <string>
#include "augs/graphics/rgba.h"
#include "augs/templates/string_templates.h"

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

	inline auto to_lua_value(const std::wstring r) {
		return to_string(r);
	}

	template <class I>
	void from_lua_value(I& in, std::wstring& r) {
		r = to_wstring(in.as<std::string>());
	}
}