#include "config_lua_table.h"
#include "game_window.h"

#include "game/bindings/bind_game_and_augs.h"
#include "augs/log.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/view/viewing_session.h"

#include "game/detail/gui/character_gui.h"

#include "generated_introspectors.h"
#include <sol.hpp>

template<class T>
static void get_config_value(
	sol::state& lua, 
	const std::string& field, 
	T& into
) {
	auto cfg_table = lua["config_table"];
	into = cfg_table[field].get<T>();
}

void config_lua_table::get_values(sol::state& lua) {
	std::string current_prefix;

	static_assert(!bind_types<can_stream_right, std::istringstream>::type<debug_drawing_settings>::value, "Trait has failed");
	static_assert(!bind_types<can_stream_right, std::istringstream>::type<debug_drawing_settings&>::value, "Trait has failed");

	augs::introspect_recursive_with_prologues <
		bind_types_t<can_stream_right, std::istringstream>,
		always_recurse,
		stop_recursion_if_valid,
		0u
	> (
		[&](const std::string& label, auto& c) {
			get_config_value(lua, current_prefix + label, c);
		},

		[&](const unsigned depth, const std::string& ss, auto...) {
			if (depth == 0) {
				current_prefix = ss + "_";
			}
		},

		[&](const unsigned depth, const std::string& ss, auto...) {
			if (depth == 0) {
				current_prefix.clear();
			}
		},

		*this
	);
}

config_lua_table::launch_type config_lua_table::get_launch_mode() const {
	return static_cast<config_lua_table::launch_type>(launch_mode);
}

input_recording_type config_lua_table::get_input_recording_mode() const {
	switch (static_cast<int>(input_recording_mode)) {
	case 0: return input_recording_type::DISABLED; break;
	case 1: return input_recording_type::LIVE_WITH_BUFFER; break;
	case 2: return input_recording_type::LIVE; break;
	default: return input_recording_type::DISABLED; break;
	}
}