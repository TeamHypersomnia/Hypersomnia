#include "augs/log.h"
#include "augs/templates/introspect.h"
#include "augs/templates/corresponding_field.h"

#include "augs/filesystem/file.h"

#include "augs/misc/lua_readwrite.h"
#include "augs/misc/script_utils.h"

#include "augs/window_framework/window.h"

#include "application/config_lua_table.h"

#include "generated/introspectors.h"

config_lua_table::config_lua_table(const augs::path_type& config_lua_path) {
	try {
		augs::load_from_lua_table(*this, config_lua_path);
	}
	catch (const augs::lua_deserialization_error err) {
		throw config_read_error(config_lua_path, err.what());
	}
}

void config_lua_table::save(const augs::path_type& target_path) const {
	augs::save_as_lua_table(*this, target_path);
}

launch_type config_lua_table::get_launch_mode() const {
	return launch_mode;
}

input_recording_type config_lua_table::get_input_recording_mode() const {
	return debug.input_recording_mode;
}