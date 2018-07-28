#pragma once
#include "augs/filesystem/file.h"
#include "augs/readwrite/lua_readwrite.h"

namespace augs {
	template <class T, class... SaverArgs>
	void save_as_lua_table(
		sol::state& lua,
		T&& object, 
		const path_type& target_path, 
		SaverArgs&&... args
	) {
		auto output_table = lua.create_named_table("my_table");
		write_lua_table(output_table, std::forward<T>(object));

		const std::string serialized_table = lua["table_to_string"](
			output_table,
			std::forward<SaverArgs>(args)...
		);

		const auto file_contents = std::string("return ") + serialized_table;
		save_as_text(target_path, file_contents);
	}

	template <class T>
	void load_from_lua_table(
		sol::state& lua,
		T& object,
		const path_type& source_path
	) {
		auto pfr = lua.do_string(
			file_to_string(source_path)
		);

		if (!pfr.valid()) {
			throw lua_deserialization_error(
				"Failed to obtain table from %x:\n%x",
				source_path,
				pfr.operator std::string()
			);
		}

		sol::table input_table = pfr;
		read_lua_table(input_table, object);
	}

	template <class T>
	T load_from_lua_table(sol::state& lua, const path_type& source_path) {
		T object{};
		load_from_lua_table<T>(lua, object, source_path);
		return object;
	}
}

