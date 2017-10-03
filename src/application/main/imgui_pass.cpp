#include "augs/misc/imgui_utils.h"
#include "augs/misc/delta.h"

#include "application/config_lua_table.h"
#include "application/gui/settings_gui.h"
#include "application/main/imgui_pass.h"

void perform_imgui_pass(
	augs::local_entropy& window_inputs,
	const vec2i screen_size,
	const augs::delta delta,
	config_lua_table& config,
	config_lua_table& last_saved_config,
	const augs::path_type& path_for_saving_config,
	settings_gui_state& settings_gui,
	sol::state& lua,
	std::function<void()> custom_imgui_logic,

	const bool ingame_menu_active,
	const bool has_gameplay_setup,

	const bool should_freeze_cursor
) {
	augs::imgui::setup_input(
		window_inputs,
		delta.in_seconds(),
		screen_size
	);

	if (should_freeze_cursor) {
		augs::imgui::neutralize_mouse();
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

	if (!ingame_menu_active) {
		custom_imgui_logic();
	}

	augs::imgui::render(config.gui_style);

	window_inputs = augs::imgui::filter_inputs(window_inputs);
}