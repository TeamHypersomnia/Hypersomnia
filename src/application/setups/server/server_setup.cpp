#include "application/setups/server/server_setup.h"
#include "application/config_lua_table.h"

server_setup::server_setup(
	sol::state& lua,
	const server_start_input& in
) {
	(void)lua;
	(void)in;
}

void server_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena server";
}

void server_setup::accept_game_gui_events(const cosmic_entropy& events) {
	control(events);
}
