#pragma once
#include <string>
#include "augs/graphics/rgba.h"
#include "augs/templates/string_templates.h"
#include "augs/filesystem/path.h"

namespace augs {
	inline auto to_lua_value(const rgba r) {
		return r.stream_to(std::ostringstream()).str();
	}

	template <class I>
	void from_lua_value(I& in, rgba& r) {
		r.from_stream(std::istringstream(in.as<std::string>()));
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
		r = in.as<std::string>();
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