#include "augs/templates/introspection_utils/introspective_equal.h"
#include "augs/templates/corresponding_field.h"

#include "augs/misc/lua/lua_utils.h"
#include "augs/readwrite/lua_file.h"
#include "augs/window_framework/window.h"

#include "application/config_lua_table.h"

config_lua_table::config_lua_table(sol::state& lua, const augs::path_type& config_lua_path) {
	try {
		augs::load_from_lua_table(lua, *this, config_lua_path);
	}
	catch (const augs::lua_deserialization_error& err) {
		throw config_read_error(config_lua_path, err.what());
	}
	catch (const augs::file_open_error& err) {
		throw config_read_error(config_lua_path, err.what());
	}
}

void config_lua_table::save(sol::state& lua, const augs::path_type& target_path) const {
	augs::save_as_lua_table(lua, *this, target_path);
}

launch_type config_lua_table::get_launch_mode() const {
	return launch_mode;
}

input_recording_type config_lua_table::get_input_recording_mode() const {
	return debug.input_recording_mode;
}

bool config_lua_table::operator==(const config_lua_table& b) const {
	return augs::introspective_equal(*this, b);
}

bool config_lua_table::operator!=(const config_lua_table& b) const {
	return !operator==(b);
}