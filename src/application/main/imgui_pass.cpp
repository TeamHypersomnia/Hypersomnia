#include "augs/misc/imgui_utils.h"
#include "augs/misc/delta.h"
#include "augs/window_framework/window.h"

#include "application/config_lua_table.h"
#include "application/gui/settings_gui.h"
#include "application/main/imgui_pass.h"

void perform_imgui_pass(
	augs::local_entropy& window_inputs,
	const configuration_subscribers dependencies,
	const augs::delta delta,
	config_lua_table& config,
	config_lua_table& last_saved_config,
	const augs::path_type& path_for_saving_config,
	settings_gui_state& settings_gui,
	sol::state& lua,
	std::function<void()> custom_imgui_logic,

	const bool ingame_menu_active,
	const bool game_gui_active,
	const bool has_gameplay_setup
) {
	/*
		Neutralize the mouse if we are within proper gameplay,
		so with when all GUIs (incl. gameplay gui) are off
	*/

	dependencies.sync_back_into(config);

	augs::imgui::setup_input(
		window_inputs,
		delta.in_seconds<double>(),
		dependencies.window.get_screen_size()
	);
	
	{
		const bool in_game_without_mouse =
			!game_gui_active
			&& has_gameplay_setup
			&& !ingame_menu_active
		;

		if (in_game_without_mouse) {
			augs::imgui::neutralize_mouse();
		}
		
		dependencies.window.set_mouse_position_frozen(in_game_without_mouse);
	}

	ImGui::NewFrame();

	/* 
		Don't show settings if we're in-game
		and a setting was specified to hide it automatically when playing
	*/

	if (
		const bool should_hide_settings =
			config.session.automatically_hide_settings_ingame
			&& has_gameplay_setup
			&& !ingame_menu_active
		;

		!should_hide_settings
	) {
		settings_gui.perform(
			lua,
			path_for_saving_config,
			config,
			last_saved_config
		);
	}

	dependencies.apply(config);

	custom_imgui_logic();

	augs::imgui::render(config.gui_style);

	window_inputs = augs::filter_inputs_for_imgui(window_inputs);
}