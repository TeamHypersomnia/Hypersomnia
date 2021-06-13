#pragma once
#include <sol/forward.hpp>

namespace augs {
	template <class Archive, class Serialized>
	void write_lua(Archive&, const Serialized& from);

	template <class Archive, class Serialized>
	void read_lua(const Archive&, Serialized& into);

	template <class Archive, class Serialized>
	void write_lua_no_overload(Archive&, const Serialized& from);

	template <class Archive, class Serialized>
	void read_lua_no_overload(const Archive&, Serialized& into);

	template <class T, class K>
	void write_table_or_field(sol::table& output_table, const T& from, K&& key);

	template <class T>
	void general_from_lua_value(const sol::object& object, T& into);
}
