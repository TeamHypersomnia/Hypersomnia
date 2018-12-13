#include "application/setups/client/client_setup.h"
#include "application/config_lua_table.h"

client_setup::client_setup(
	sol::state& lua,
	const client_start_input& in
) {
	(void)lua;
	(void)in;
}

void client_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena client";
}

void client_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

void client_setup::perform_custom_imgui(perform_custom_imgui_input) {

}

void client_setup::draw_custom_gui(const draw_setup_gui_input& in) {
	(void)in;

}

