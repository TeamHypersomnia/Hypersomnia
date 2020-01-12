#pragma once
#include <functional>
#include "augs/filesystem/path.h"
#include "augs/misc/machine_entropy.h"

class settings_gui_state;
struct config_lua_table;

namespace sol {
	class state;
}

namespace augs {
	class delta;
}

void perform_imgui_pass(
	augs::local_entropy& window_inputs,
	const vec2i screen_size,
	const augs::delta delta,
	const config_lua_table& canon_config,
	config_lua_table& config,
	config_lua_table& last_saved_config,
	const augs::path_type& path_for_saving_config,
	settings_gui_state& settings_gui,
	const augs::audio_context& audio,
	sol::state& lua,
	std::function<void()> custom_imgui_logic,
	std::function<void()> custom_imgui_logic_hide_in_menu,

	const bool ingame_menu_active,
	const bool has_gameplay_setup,
	const bool should_freeze_cursor,

	const bool float_tests_succeeded
);