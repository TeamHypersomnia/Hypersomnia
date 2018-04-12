#pragma once
#include <string>
#include "3rdparty/imgui/imgui.h"
#include "augs/graphics/rgba.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/filesystem/path.h"

#if LUA_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

namespace augs {
	inline auto to_lua_value(const rgba r) {
		auto s = std::ostringstream();
		r.stream_to(s);
		return s.str();
	}

	template <class I>
	void from_lua_value(I& in, rgba& r) {
		auto s = std::istringstream(in.template as<std::string>());
		r.from_stream(s);
	}

	inline auto to_lua_value(const path_type r) {
		return to_forward_slashes(r.string());
	}

	template <class I>
	void from_lua_value(I& in, path_type& r) {
		r = in.template as<std::string>();
	}

	inline auto to_lua_value(const ImVec4& r) {
		return to_lua_value(rgba(r));
	}

	template <class I>
	void from_lua_value(I& in, ImVec4& v) {
		rgba r;
		auto s = std::istringstream(in.template as<std::string>());
		r.from_stream(s);
		v = r;
	}
}
