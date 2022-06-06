#include "augs/templates/container_templates.h"
#include "application/setups/debugger/editor_recent_paths.h"
#include "application/setups/debugger/editor_paths.h"
#include "augs/readwrite/lua_file.h"

editor_recent_paths::editor_recent_paths(sol::state& lua) {
	try {
		augs::load_from_lua_table(lua, *this, get_recent_paths_path());
	}
	catch (...) {

	}
}

void editor_recent_paths::add(sol::state& lua, augs::path_type path) {
	erase_element(paths, path);
	paths.insert(paths.begin(), path);
	augs::save_as_lua_table(lua, *this, get_recent_paths_path());
}

void editor_recent_paths::clear(sol::state& lua) {
	paths.clear();
	augs::save_as_lua_table(lua, *this, get_recent_paths_path());
}

bool editor_recent_paths::empty() const {
	return paths.empty();
}
