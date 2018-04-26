#pragma once
#include <sol/forward.hpp>

namespace augs {
	template <class Serialized>
	void write_lua(sol::table output_table, const Serialized& from);

	template <class Serialized>
	void read_lua(sol::table input_table, Serialized& into);

	template <class Serialized>
	void read_lua(sol::table input_table, Serialized& into);

	template <class A, class B, class Serialized>
	void read_lua(sol::proxy<A, B> input_proxy, Serialized& into);
}