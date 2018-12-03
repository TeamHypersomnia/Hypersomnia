#include "application/setups/server/server_setup.h"
#include "application/config_lua_table.h"

#include "application/network/network_adapters.hpp"

/* To avoid incomplete type error */
server_setup::~server_setup() = default;

double server_setup::get_current_time() {
	return yojimbo_time();
}

server_setup::server_setup(
	sol::state& lua,
	const server_start_input& in
) : 
	server(std::make_unique<server_adapter>(in)),
	server_time(yojimbo_time())
{
	(void)lua;
}

void server_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena server";
}

void server_setup::accept_game_gui_events(const game_gui_entropy_type&) {

}

template <class M>
constexpr bool is_server_specific_v = is_one_of_v<
	net_messages::initial_solvable,
	net_messages::step_entropy
>;

void server_setup::advance_internal(const setup_advance_input& in) {
	(void)in;

	auto message_callback = [&](
		const client_id_type id, 
		const auto& message
	) {
		using M = remove_cref<decltype(message)>;

		if constexpr(is_server_specific_v<M>) {
			/* What are you trying to pull off? */
			server->deal_with_malicious_client(id);
		}
		else {

		}
	};

	server->advance(server_time, message_callback);
}

double server_setup::get_inv_tickrate() const {
	return std::visit(
		[&](const auto& typed_mode) {
			using M = remove_cref<decltype(typed_mode)>;

			if constexpr(supports_mode_v<M>) {
				return typed_mode.round_speeds.calc_inv_tickrate();
			}
			else {
				ensure(false && "Unsupported mode on the server!");
				return 1 / 60.0;
			}
		},
		current_mode
	);
}
