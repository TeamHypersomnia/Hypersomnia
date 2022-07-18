#pragma once
#include <string>
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

#if JSON_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

namespace augs {
	template <class T>
	inline void to_json_value(T& out, const rgba& from) {
		out.StartArray();
		out.Uint(from.r);
		out.Uint(from.g);
		out.Uint(from.b);
		out.Uint(from.a);
		out.EndArray();
	}

	template <class T>
	void from_json_value(T& from, rgba& out) {
		if (from.IsArray()) {
			if (from.Size() == 4 && from[0].IsUint() && from[1].IsUint() && from[2].IsUint() && from[3].IsUint()) {
				out.set(from[0].GetUint(), from[1].GetUint(), from[2].GetUint(), from[3].GetUint());
			}
		}
	}

	template <class T>
	inline void to_json_value(T& out, const vec2& from) {
		out.StartArray();
		out.Double(from.x);
		out.Double(from.y);
		out.EndArray();
	}

	template <class I>
	void from_json_value(I& from, vec2& out) {
		if (from.IsArray() && from.Size() == 2 && from[0].IsFloat() && from[1].IsFloat()) {
			out.set(from[0].GetFloat(), from[1].GetFloat());
		}
	}
}
