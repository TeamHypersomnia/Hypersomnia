#include "application/setups/server/server_setup.h"
#include "application/config_lua_table.h"
#include "application/arena/arena_utils.h"

#include "application/network/network_adapters.hpp"

#include "augs/filesystem/file.h"
#include "application/arena/arena_paths.h"

/* To avoid incomplete type error */
server_setup::~server_setup() = default;

mode_player_id server_setup::to_mode_player_id(const client_id_type& id) {
	mode_player_id out;
	out.value = static_cast<mode_player_id::id_value_type>(id);

	return out;
}

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

bool server_setup::add_to_arena(add_to_arena_input) {
	return true;
}

void server_setup::disconnect_and_unset(const client_id_type& id) {
	server->disconnect_client(id);
	clients[id].unset();
}

void server_setup::advance_clients_state(const setup_advance_input& in) {
	(void)in;

	/* Do it only once per tick */
	bool added_someone_already = false;
	bool removed_someone_already = false;

	for (auto& c : clients) {
		if (!c.is_set()) {
			continue;
		}

		const auto client_id = static_cast<client_id_type>(index_in(clients, c));

		auto add_client_if_not_yet_in_mode = [&]() {
			const auto mode_id = to_mode_player_id(client_id);

			auto final_nickname = on_mode(
				[&](const auto& typed_mode) -> std::string {
					if (nullptr == typed_mode.find(mode_id)) {
						auto nickname = std::string(c.settings.chosen_nickname);

						while (typed_mode.find_player_by(nickname)) {
							nickname += std::to_string(client_id);
						}

						return nickname;
					}

					return "";
				}
			);

			if (final_nickname.size() > 0) {
				mode_entropy_general cmd;

				cmd.added_player = add_player_input {
					mode_id,
					std::move(final_nickname),
					faction_type::SPECTATOR
				};

				total_collected.control(cmd);
				added_someone_already = true;
			}
		};

		auto kick_if_afk = [&](){
			const auto mode_id = to_mode_player_id(client_id);

			if (c.should_kick_due_to_inactivity(vars, server_time)) {
				disconnect_and_unset(client_id);

				mode_entropy_general cmd;
				cmd.removed_player = mode_id;

				total_collected.control(cmd);
				removed_someone_already = true;
			}
		};

		if (!added_someone_already) {
			if (c.state != server_client_state::type::PENDING_WELCOME) {
				add_client_if_not_yet_in_mode();
			}
		}

		if (!removed_someone_already) {
			kick_if_afk();
		}
	}
}

void server_setup::handle_client_messages(const setup_advance_input& in) {
	(void)in;

	auto unset_client = [&](const client_id_type& id) {
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
			unset_client(id);
			return message_handler_result::ABORT_AND_DISCONNECT;
		};

		if constexpr(std::is_same_v<M, connection_event_type>) {
			const auto type = message;

			if (type == connection_event_type::CONNECTED) {
				clients[id] = server_client_state(server_time);

				LOG("Client connected. Details:\n%x", describe_client(id));
			}
			else {
				unset_client(id);
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
					// TODO: check if this is handled when we connect with < 3 characters as nickname
					auto message_provider = [&](net_messages::initial_steps& msg) {
						(void)msg;
					};

					server->send_message(id, game_channel_type::SOLVABLE_STREAM, message_provider);

					c.state = S::RECEIVING_INITIAL_STATE;
				}

				/* A client might re-state its requested settings */

				c.settings = static_cast<const requested_client_settings&>(message);
				c.last_valid_activity_time = server_time;
			}
			else if constexpr (std::is_same_v<M, N::client_entropy>) {
				if (c.state != S::IN_GAME) {
					return stop_processing_this_client();
				}

				c.last_valid_activity_time = server_time;
			}
			else {
				static_assert(always_false_v<M>, "Unhandled message type.");
			}
		}

		return message_handler_result::CONTINUE;
	};

	server->advance(server_time, message_callback);

	{
		auto& ticks_remaining = ticks_until_sending_packets;

		if (ticks_remaining == 0) {
			server->send_packets();

			ticks_remaining = vars.send_updates_once_every_tick;
			--ticks_remaining;
		}
	}
}

void server_setup::advance_internal(const setup_advance_input& in) {
	(void)in;

	handle_client_messages(in);
	advance_clients_state(in);
}

double server_setup::get_inv_tickrate() const {
	return on_mode(
		[&](const auto& typed_mode) {
			return typed_mode.round_speeds.calc_inv_tickrate();
		}
	);
}
