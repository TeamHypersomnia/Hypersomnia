#pragma once
#include "augs/filesystem/file.h"
#include "augs/readwrite/lua_readwrite.h"

namespace augs {
	template <class T, class... SaverArgs>
	auto to_lua_string(
		sol::state& lua,
		T&& object, 
		SaverArgs&&... args
	) {
		auto output_table = lua.create_named_table("my_table");
		write_lua(output_table, std::forward<T>(object));

		const std::string serialized_table = lua["table_to_string"](
			output_table,
			std::forward<SaverArgs>(args)...
		);

		return std::string("return ") + serialized_table;
	}

	template <class T, class... SaverArgs>
	void save_as_lua_table(
		sol::state& lua,
		T&& object, 
		const path_type& target_path, 
		SaverArgs&&... args
	) {
		save_as_text(
			target_path, 
			to_lua_string(lua, object, std::forward<SaverArgs>(args)...)
		);
	}

	template <class T, class... SaverArgs>
	void save_as_lua_patch(
		sol::state& lua,
		T&& source, 
		T&& target, 
		const path_type& target_path, 
		SaverArgs&&... args
	) {
		auto source_table = lua.create_named_table("source_table");
		write_lua(source_table, std::forward<T>(source));

		auto target_table = lua.create_named_table("target_table");
		write_lua(target_table, std::forward<T>(target));

		const std::string serialized_table = lua["table_diff_to_string"](
			source_table,
			target_table,
			std::forward<SaverArgs>(args)...
		);

		const auto table_file_contents = std::string("return ") + serialized_table;

		save_as_text(
			target_path, 
			table_file_contents
		);
	}

	template <class T>
	void load_from_lua_patch(
		sol::state& lua,
		T& object,
		const path_type& patch_path
	) {
		auto source_table = lua.create_named_table("source_table");
		write_lua(source_table, std::forward<T>(object));

		auto pfr = lua.do_string(
			file_to_string(patch_path)
		);

		if (!pfr.valid()) {
			throw lua_deserialization_error(
				"Failed to obtain patch table from %x:\n%x",
				patch_path,
				pfr.operator std::string()
			);
		}

		sol::table patch_table = pfr;

		auto patched_table = lua["patch_table"](
			source_table,
			patch_table
		);

		read_lua(patched_table, object);
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
		read_lua(input_table, object);
	}

	template <class T>
	T load_from_lua_table(sol::state& lua, const path_type& source_path) {
		T object{};
		load_from_lua_table<T>(lua, object, source_path);
		return object;
	}
}

