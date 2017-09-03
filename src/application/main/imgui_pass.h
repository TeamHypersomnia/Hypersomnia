#pragma once
#include <functional>
#include "augs/filesystem/path.h"
#include "augs/misc/machine_entropy.h"

class settings_gui_state;
struct configuration_subscribers;
struct config_lua_table;

void perform_imgui_pass(
	augs::local_entropy& window_inputs,
	const configuration_subscribers dependencies,
	const float delta_seconds,
	config_lua_table& config,
	config_lua_table& last_saved_config,
	const augs::path_type& path_for_saving_config,
	settings_gui_state& settings_gui,
	std::function<void()> custom_imgui_logic,
	const bool ingame_menu_active,
	const bool game_gui_active,
	const bool has_gameplay_setup
);