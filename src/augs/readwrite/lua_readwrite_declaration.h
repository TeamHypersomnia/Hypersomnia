#pragma once

namespace augs {
	template <class Table, class Serialized>
	void write_lua_table(Table output_table, const Serialized& from);

	template <class Table, class Serialized>
	void read_lua_table(Table input_table, Serialized& into);
}