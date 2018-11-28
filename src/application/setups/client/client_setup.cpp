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

void client_setup::accept_game_gui_events(const cosmic_entropy& events) {
	control(events);
}
