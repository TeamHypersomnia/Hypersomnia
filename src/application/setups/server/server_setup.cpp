#include "augs/misc/compress.h"

#include "application/setups/server/server_setup.h"
#include "application/config_lua_table.h"
#include "application/arena/arena_utils.h"

#include "application/network/network_adapters.hpp"

#include "augs/filesystem/file.h"
#include "application/arena/arena_paths.h"

#include "application/network/net_message_translation.h"
#include "application/network/net_message_serialization.h"

#include "augs/misc/pool/pool_io.hpp"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"

/* To avoid incomplete type error */
server_setup::~server_setup() = default;

mode_player_id server_setup::to_mode_player_id(const client_id_type& id) {
	mode_player_id out;
	out.value = static_cast<mode_player_id::id_value_type>(id);

	return out;
}

void server_setup::log_malicious_client(const client_id_type id) {
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

void server_setup::init_client(const client_id_type& id) {
	clients[id] = server_client_state(server_time);

	LOG("Client connected. Details:\n%x", describe_client(id));
}

void server_setup::unset_client(const client_id_type& id) {
	LOG("Client disconnected. Details:\n%x", describe_client(id));
	clients[id].unset();
}

void server_setup::disconnect_and_unset(const client_id_type& id) {
	server->disconnect_client(id);
	unset_client(id);
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

		using S = server_client_state::type;

		if (c.state == S::RECEIVING_INITIAL_STATE) {
			if (!server->has_messages_to_send(client_id, game_channel_type::SOLVABLE_STREAM)) {
#if 0
				auto message_provider = [&](net_messages::initial_steps_correction& msg) {
					(void)msg;
				};

				server->send_message(client_id, game_channel_type::SOLVABLE_STREAM, message_provider);

				c.state = S::RECEIVING_INITIAL_STATE_CORRECTION;
#else
				// TODO: Actually send the correction
				c.set_in_game(server_time);
#endif
			}
		}
		else if (c.state == S::RECEIVING_INITIAL_STATE_CORRECTION) {
			if (!server->has_messages_to_send(client_id, game_channel_type::SOLVABLE_STREAM)) {
				c.set_in_game(server_time);
			}
		}
	}
}

template <class T>
constexpr bool payload_easily_movable_v = true;

template <class T, class F>
message_handler_result server_setup::handle_client_message(
	const client_id_type& client_id, 
	F&& read_payload
) {
	constexpr auto abort_v = message_handler_result::ABORT_AND_DISCONNECT;
	constexpr bool is_easy_v = payload_easily_movable_v<T>;

	std::conditional_t<is_easy_v, T, std::monostate> payload;

	if constexpr(is_easy_v) {
		if (!read_payload(payload)) {
			return abort_v;
		}
	}

	using S = server_client_state::type;
	namespace N = net_messages;

	auto& c = clients[client_id];
	ensure(c.is_set());

	if constexpr (std::is_same_v<T, requested_client_settings>) {
		/* 
			A client might re-state its requested settings
			even outside of the PENDING_WELCOME state
		*/

		c.settings = std::move(payload);

		if (c.state == S::PENDING_WELCOME) {
			server->send_payload(
				client_id, 
				game_channel_type::SOLVABLE_STREAM, 

				buffers,

				initial_arena_state_payload<true> {
					scene.world.get_solvable().significant,
					current_mode,
					vars
				}
			);

			c.state = S::RECEIVING_INITIAL_STATE;
		}
	}
	else if constexpr (std::is_same_v<T, total_mode_player_entropy>) {
		if (c.state != S::IN_GAME) {
			/* 
				The client shall not send commands until it receives the first step entropy,
				at which point being IN_GAME is guaranteed.

				Actually, wouldn't the ack for the last fragment come along the first client commands?
			*/

			return abort_v;
		}
	}
	else {
		static_assert(always_false_v<T>, "Unhandled payload type.");
	}

	c.last_valid_activity_time = server_time;
	return message_handler_result::CONTINUE;
}

void server_setup::handle_client_messages(const setup_advance_input& in) {
	(void)in;

	server->advance(server_time, *this);

	++current_step;

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
