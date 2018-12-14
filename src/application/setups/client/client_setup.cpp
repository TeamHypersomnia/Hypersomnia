#include "application/setups/client/client_setup.h"
#include "application/config_lua_table.h"

#include "application/network/client_adapter.hpp"

client_setup::client_setup(
	sol::state& lua,
	const client_start_input& in
) {
	(void)lua;
	(void)in;
}

client_setup::~client_setup() {
	client->get_specific().Disconnect();
}

net_time_t client_setup::get_current_time() {
	return yojimbo_time();
}

entity_id client_setup::get_viewed_character_id() const {
	return get_arena_handle().on_mode(
		[&](const auto& typed_mode) {
			return typed_mode.lookup(get_local_player_id());
		}
	);
}

void client_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena client";
}

void client_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

online_arena_handle<false> client_setup::get_arena_handle() {
	return get_arena_handle_impl<online_arena_handle<false>>(*this);
}

online_arena_handle<true> client_setup::get_arena_handle() const {
	return get_arena_handle_impl<online_arena_handle<true>>(*this);
}

double client_setup::get_inv_tickrate() const {
	return get_arena_handle().get_inv_tickrate();
}

double client_setup::get_audiovisual_speed() const {
	return get_arena_handle().get_audiovisual_speed();
}
