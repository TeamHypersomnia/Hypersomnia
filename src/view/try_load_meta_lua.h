#pragma once
#include "augs/filesystem/path.h"
#include "augs/readwrite/lua_file.h"

inline auto get_meta_lua_path(augs::path_type resolved) {
	return resolved.replace_extension(".meta.lua");
}

template <class T>
void try_load_meta_lua(sol::state& lua, T& meta, augs::path_type resolved) {
	try {
		augs::load_from_lua_table(lua, meta, get_meta_lua_path(resolved));
	}
	catch (const augs::file_open_error& err) {
		/* Do not intervene. */
	}
}


template <class T>
void save_meta_lua(sol::state& lua, const T& meta, augs::path_type resolved) {
	augs::save_as_lua_table(lua, meta, get_meta_lua_path(resolved));
}
