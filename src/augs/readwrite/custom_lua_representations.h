#pragma once
#include <string>
#include "augs/graphics/rgba.h"
#include "augs/templates/string_templates.h"
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
		auto nice_representation = r.string();

		for (auto& s : nice_representation) {
			/* Double backslash is ugly */

			if (s == '\\') {
				s = '/';
			}
		}

		return nice_representation;
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

	inline auto to_lua_value(const std::wstring r) {
		return to_string(r);
	}

	template <class I>
	void from_lua_value(I& in, std::wstring& r) {
		r = to_wstring(in.template as<std::string>());
	}
}