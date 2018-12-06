#include "application/setups/server/server_setup.h"
#include "application/config_lua_table.h"
#include "application/arena/arena_utils.h"

#include "application/network/network_adapters.hpp"

#include "augs/filesystem/file.h"
#include "application/arena/arena_paths.h"

/* To avoid incomplete type error */
server_setup::~server_setup() = default;

void server_setup::deal_with_malicious_client(const client_id_type id) {
	LOG("Malicious client detected. Details:\n%x", describe_client(id));
}

std::string server_setup::describe_client(const client_id_type id) const {
	return typesafe_sprintf(
		"Id: %x\nNickname: %x",
		id,
		"not implemented"
	);
}

net_time_t server_setup::get_current_time() {
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

	/* Force the loading of a new arena the next time server vars are applied. */
	vars.current_arena.clear();
}

void server_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena server";
}

void server_setup::apply(const config_lua_table& cfg) {
	const auto& new_vars = cfg.server;

	if (vars.current_arena != new_vars.current_arena) {
		try {
			choose_arena(new_vars.current_arena);
		}
		catch (const augs::file_open_error& err) {
			/* 
				TODO!!! 
				In case of loading errors, we should update the config to the previously working map (or empty string). 
				We should also remove this line: "vars = cfg.server;" since it will update the arena name despite not having it loaded.
			*/

			/* This should never really happen as we'll always check before allowing admin to set a map name. */
			LOG("Arena named %x was not found on the server!");
			ensure(false);
		}
	}

	vars = cfg.server;
}

void server_setup::choose_arena(const std::string& name) {
	if (name.empty())  {
		scene.clear();
		rulesets = {};
	}
	else {
		load_arena_from(
			arena_paths(name),
			scene,
			rulesets
		);

		if (vars.override_default_ruleset.empty()) {
			current_mode.choose(rulesets.meta.server_default);
		}
		else {
			ensure(false && "Not implemented!");
		}
	}

	vars.current_arena = name;
}

void server_setup::accept_game_gui_events(const game_gui_entropy_type&) {

}

bool server_setup::add_to_arena(const client_id_type& client_id, const net_messages::client_welcome& msg) {
	(void)client_id;
	(void)msg;

	return true;
}

void server_setup::advance_internal(const setup_advance_input& in) {
	(void)in;

	auto do_disconnect_client = [&](const client_id_type& id) {
		if (auto& c = clients[id]; c.is_set()) {
			/* 
				A client might have disconnected during message processing. 
				The disconnection is handled right away so that the client array NEVER EVER 
				holds lingering state for already disconnected clients.

				However, a disconnection event was still posted to pending events, due to calling disconnect_client.

				We don't want to process the disconnection twice so we must first check if the client was already set.
			*/

			LOG("Client disconnected. Details:\n%x", describe_client(id));
			c.unset();
		}
	};

	auto message_callback = [&](
		const client_id_type id, 
		const auto& message
	) {
		using M = remove_cref<decltype(message)>;

		auto stop_processing_this_client = [&]() {
			do_disconnect_client(id);
			return callback_result::ABORT;
		};

		if constexpr(std::is_same_v<M, connection_event_type>) {
			const auto type = message;

			if (type == connection_event_type::CONNECTED) {
				clients[id] = server_client_state(server_time);

				LOG("Client connected. Details:\n%x", describe_client(id));
			}
			else {
				do_disconnect_client(id);
			}
		}
		else {
			ensure(server->is_client_connected(id));

			auto& c = clients[id];
			using S = server_client_state::type;

			namespace N = net_messages;

			if constexpr(!M::client_to_server) {
				static_assert(M::server_to_client);
				/* What are you trying to pull off? */
				deal_with_malicious_client(id);
				return stop_processing_this_client();
			}
			else if constexpr (std::is_same_v<M, N::client_welcome>) {
				if (c.state == S::PENDING_WELCOME) {
					if (!add_to_arena(id, message)) {
						return stop_processing_this_client();
					}

					c.state = S::RECEIVING_INITIAL_STATE;
				}
			}
			else if constexpr (std::is_same_v<M, N::client_entropy>) {
				if (c.state != S::IN_GAME) {
					// TODO: ensure that receiving initial state has ended
					return stop_processing_this_client();
				}
			}
			else {
				static_assert(always_false_v<M>, "Unhandled message type.");
			}
		}

		return callback_result::CONTINUE;
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
		current_mode.state
	);
}
