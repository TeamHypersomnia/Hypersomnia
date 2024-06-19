#pragma once
#include <string>
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"
#include "rapidjson/prettywriter.h"

#if JSON_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

struct ImVec2;
struct ImVec4;

namespace augs {
	template <class T>
	inline void to_json_value(T& out, const ImVec2& in) {
		auto* from = reinterpret_cast<const float*>(&in);

		out.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		out.StartArray();
		out.Double(from[0]);
		out.Double(from[1]);
		out.EndArray();
		out.SetFormatOptions(rapidjson::kFormatDefault);
	}

	template <class T>
	void from_json_value(T& from, ImVec2& in) {
		auto* out = reinterpret_cast<float*>(&in);

		if (from.IsArray()) {
			if (from.Size() == 2 && from[0].IsNumber() && from[1].IsNumber()) {
				out[0] = from[0].GetFloat();
				out[1] = from[1].GetFloat();
			}
		}
	}

	template <class T>
	inline void to_json_value(T& out, const rgba& from) {
		out.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		out.StartArray();
		out.Uint(from.r);
		out.Uint(from.g);
		out.Uint(from.b);
		out.Uint(from.a);
		out.EndArray();
		out.SetFormatOptions(rapidjson::kFormatDefault);
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
		out.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		out.StartArray();
		out.Double(from.x);
		out.Double(from.y);
		out.EndArray();
		out.SetFormatOptions(rapidjson::kFormatDefault);
	}

	template <class I>
	void from_json_value(I& from, vec2& out) {
		if (from.IsArray() && from.Size() == 2 && from[0].IsNumber() && from[1].IsNumber()) {
			out.set(from[0].GetFloat(), from[1].GetFloat());
		}
	}

	template <class T>
	inline void to_json_value(T& out, const vec2d& from) {
		out.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		out.StartArray();
		out.Double(from.x);
		out.Double(from.y);
		out.EndArray();
		out.SetFormatOptions(rapidjson::kFormatDefault);
	}

	template <class I>
	void from_json_value(I& from, vec2d& out) {
		if (from.IsArray() && from.Size() == 2 && from[0].IsNumber() && from[1].IsNumber()) {
			out.set(from[0].GetDouble(), from[1].GetDouble());
		}
	}

	template <class T>
	inline void to_json_value(T& out, const vec2i& from) {
		out.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		out.StartArray();
		out.Int(from.x);
		out.Int(from.y);
		out.EndArray();
		out.SetFormatOptions(rapidjson::kFormatDefault);
	}

	template <class I>
	void from_json_value(I& from, vec2i& out) {
		if (from.IsArray() && from.Size() == 2) {
			if (from[0].IsInt() && from[1].IsInt()) {
				out.set(from[0].GetInt(), from[1].GetInt());
			}
			else {
				if (from[0].IsNumber() && from[1].IsNumber()) {
					out.set(static_cast<int>(from[0].GetDouble()), static_cast<int>(from[1].GetDouble()));
				}
			}
		}
	}

	template <class T>
	inline void to_json_value(T& out, const vec2u& from) {
		out.StartArray();
		out.Uint(from.x);
		out.Uint(from.y);
		out.EndArray();
	}

	template <class I>
	void from_json_value(I& from, vec2u& out) {
		if (from.IsArray() && from.Size() == 2 && from[0].IsUint() && from[1].IsUint()) {
			if (from[0].IsUint() && from[1].IsUint()) {
				out.set(from[0].GetUint(), from[1].GetUint());
			}
			else {
				if (from[0].IsNumber() && from[1].IsNumber()) {
					out.set(static_cast<unsigned>(from[0].GetDouble()), static_cast<unsigned>(from[1].GetDouble()));
				}
			}
		}
	}

	template <class T>
	inline void to_json_value(T& out, const ImVec4& in) {
		auto* from = reinterpret_cast<const float*>(&in);

		to_json_value(out, rgba(vec4{from[0], from[1], from[2], from[3]}));
	}

	template <class T>
	void from_json_value(T& from, ImVec4& in) {
		auto* out = reinterpret_cast<float*>(&in);

		if (from.IsArray()) {
			if (from.Size() == 4 && from[0].IsNumber() && from[1].IsNumber() && from[2].IsNumber() && from[3].IsNumber()) {
				rgba rr;
				from_json_value(from, rr);
				auto v = rr.operator vec4();

				out[0] = v[0];
				out[1] = v[1];
				out[2] = v[2];
				out[3] = v[3];
			}
		}
	}
}
