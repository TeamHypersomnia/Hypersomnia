#include "config_lua_table.h"
#include "game_window.h"

#include <luabind/luabind.hpp>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/log.h"

#include "augs/templates/string_templates.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/view/viewing_session.h"

#include "game/detail/gui/character_gui.h"
#include "augs/scripting/lua_state_raii.h"

template<class T>
static void get_config_value(augs::lua_state_raii& lua, const std::string& field, T& into) {
	into = luabind::object_cast<T>(luabind::globals(lua)["config_table"][field]);
}

static void get_config_value(augs::lua_state_raii& lua, const std::string& field, bool& into) {
	into = luabind::object_cast<double>(luabind::globals(lua)["config_table"][field]) > 0.0;
}

void config_lua_table::get_values(augs::lua_state_raii& lua) {
	augs::introspect(
		*this,
		[&](auto& c, const std::string& ss) {
			get_config_value(lua, replace_all(ss, ".", "_"), c);
		}
	);
}

config_lua_table::launch_type config_lua_table::get_launch_mode() const {
	switch (static_cast<int>(launch_mode)) {
	case 0: return launch_type::MAIN_MENU; break;
	case 1: return launch_type::LOCAL; break;
	case 2: return launch_type::LOCAL_DETERMINISM_TEST; break;
	case 3: return launch_type::DIRECTOR; break;
	case 4: return launch_type::ONLY_CLIENT; break;
	case 5: return launch_type::ONLY_SERVER; break;
	case 6: return launch_type::CLIENT_AND_SERVER; break;
	case 7: return launch_type::TWO_CLIENTS_AND_SERVER; break;
	default: return launch_type::INVALID; break;
	}
}

input_recording_type config_lua_table::get_input_recording_mode() const {
	switch (static_cast<int>(input_recording_mode)) {
	case 0: return input_recording_type::DISABLED; break;
	case 1: return input_recording_type::LIVE_WITH_BUFFER; break;
	case 2: return input_recording_type::LIVE; break;
	default: return input_recording_type::DISABLED; break;
	}
}