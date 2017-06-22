#include "generated/introspectors.h"

#include "augs/log.h"
#include "augs/misc/lua_readwrite.h"
#include "augs/templates/introspect.h"

#include "application/game_window.h"
#include "application/config_lua_table.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/view/viewing_session.h"
#include "game/detail/gui/character_gui.h"

#include "augs/misc/script_utils.h"
#include "augs/filesystem/file.h"

static_assert(is_introspective_leaf_v<launch_type>);
static_assert(has_enum_to_string_v<launch_type>);

config_lua_table::config_lua_table(
	const std::string& config_lua_path, 
	const std::string& config_local_lua_path
) {
	std::string used_filename = config_lua_path;

	if (augs::file_exists(config_local_lua_path)) {
		used_filename = config_local_lua_path;
	}

	augs::load_from_lua_table(*this, used_filename);
}

static_assert(has_enum_to_string_v<launch_type>);
static_assert(has_for_each_enum_v<launch_type>);
static_assert(has_for_each_enum_v<input_recording_type>);

static_assert(is_container_v<decltype(input_context::key_to_intent)>);

void config_lua_table::save(const std::string& target_path) const {
	augs::save_as_lua_table(*this, target_path);
}

launch_type config_lua_table::get_launch_mode() const {
	return launch_mode;
}

input_recording_type config_lua_table::get_input_recording_mode() const {
	return input_recording_mode;
}