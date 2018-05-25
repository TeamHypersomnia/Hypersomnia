#pragma once
#include "augs/filesystem/path.h"
#include "augs/readwrite/lua_file.h"

template <class T>
void try_load_meta_lua(sol::state& lua, T& meta, augs::path_type resolved) {
	try {
		const auto meta_path = augs::path_type(resolved).replace_extension(".meta.lua");
		augs::load_from_lua_table(lua, meta, meta_path);
	}
	catch (augs::file_open_error err) {
		/* Do not intervene. */
	}
}
