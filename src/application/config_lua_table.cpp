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

static_assert(is_introspective_leaf_v<launch_type>);

void config_lua_table::get_values(sol::state& lua) {
	sol::table config_table = lua["config_table"];
	ensure(config_table.valid());

	augs::read(config_table, *this);
}

launch_type config_lua_table::get_launch_mode() const {
	return launch_mode;
}

input_recording_type config_lua_table::get_input_recording_mode() const {
	return input_recording_mode;
}