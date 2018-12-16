#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/compress.h"

#include "application/setups/server/server_setup.h"
#include "application/config_lua_table.h"
#include "application/arena/arena_utils.h"

#include "application/network/server_adapter.hpp"

#include "augs/filesystem/file.h"
#include "application/arena/arena_paths.h"

#include "application/network/net_message_translation.h"
#include "application/network/net_message_serialization.h"

#include "augs/misc/pool/pool_io.hpp"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"

#include "application/arena/arena_handle.h"
#include "application/arena/choose_arena.h"

/* To avoid incomplete type error */
server_setup::~server_setup() {
	server->stop();
}

server_setup::server_setup(
	sol::state& lua,
	const server_start_input& in,
	const server_vars& initial_vars
) : 
	lua(lua),
	last_start(in),
	server(std::make_unique<server_adapter>(in)),
	server_time(yojimbo_time())
{
	const bool force = true;
	apply(initial_vars, force);
}

mode_player_id server_setup::get_admin_player_id() const {
	return mode_player_id::machine_admin();
}

mode_player_id server_setup::to_mode_player_id(const client_id_type& id) {
	mode_player_id out;
	out.value = static_cast<mode_player_id::id_value_type>(id);

	return out;
}

online_arena_handle<false> server_setup::get_arena_handle() {
	return get_arena_handle_impl<online_arena_handle<false>>(*this);
}

online_arena_handle<true> server_setup::get_arena_handle() const {
	return get_arena_handle_impl<online_arena_handle<true>>(*this);
}

entity_id server_setup::get_viewed_character_id() const {
	return get_arena_handle().on_mode(
		[&](const auto& typed_mode) {
			return typed_mode.lookup(get_admin_player_id());
		}
	);
}

void server_setup::log_malicious_client(const client_id_type id) {
	LOG("Malicious client detected. Details:\n%x", describe_client(id));

#if !IS_PRODUCTION_BUILD
	ensure(false && "Client has sent some invalid data.");
#endif
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

void server_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena server";
}

void server_setup::apply(const server_vars& new_vars, const bool force) {
	if (force || vars.current_arena != new_vars.current_arena) {
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

	vars = new_vars;
}

void server_setup::apply(const config_lua_table& cfg) {
	const bool force = false;
	apply(cfg.server, force);
}

void server_setup::choose_arena(const std::string& name) {
	::choose_arena(
		lua,
		get_arena_handle(),
		vars,
		initial_signi
	);

	vars.current_arena = name;

	{
		mode_entropy_general cmd;

		cmd.added_player = add_player_input {
			get_admin_player_id(),
			vars.admin_nickname,
			faction_type::SPECTATOR
		};

		local_collected.clear();
		local_collected.control(cmd);
	}
}

void server_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

void server_setup::init_client(const client_id_type& id) {
	clients[id].init(server_time);

	LOG("Client connected. Details:\n%x", describe_client(id));
}

void server_setup::unset_client(const client_id_type& id) {
	LOG("Client disconnected. Details:\n%x", describe_client(id));
	clients[id].unset();
}

void server_setup::disconnect_and_unset(const client_id_type& id) {
	ensure(false);
	server->disconnect_client(id);
	unset_client(id);
}

void server_setup::accept_entropy_of_client(
	const mode_player_id mode_id,
	const total_client_entropy& entropy
) {
	if (!entropy.empty()) {
		const auto client_entity_id = get_arena_handle().on_mode(
			[&](const auto& typed_mode) {
				if (const auto p = typed_mode.find(mode_id)) {
					return p->controlled_character_id;
				}

				return entity_id::dead();
			}
		);

		/* 
			TODO SECURITY: validate the entropies for unauthorized item transfers! 
			Remember, though, that the client could have mistakenly thought that they are alive.
			Therefore we should not assume they are malicious and kick them, but just ignore the invalid commands.
		*/

		step_collected.accumulate(
			mode_id,
			client_entity_id,
			entropy
		);
	}
	else {

	}
}

void server_setup::advance_clients_state() {
	/* Do it only once per tick */
	bool added_someone_already = false;
	bool removed_someone_already = false;

	const auto inv_simulation_delta_ms = 1.0 / (get_inv_tickrate() * 1000.0);

	auto in_steps = [inv_simulation_delta_ms](const auto ms) {
		return ms * inv_simulation_delta_ms;
	};

	constexpr auto max_pending_commands_for_client_v = static_cast<uint8_t>(255);

	auto character_exists_for = [&](const mode_player_id mode_id) {
		return get_arena_handle().on_mode(
			[&](const auto& typed_mode) {
				return typed_mode.find(mode_id);
			}
		);
	};
	
	for (auto& c : clients) {
		using S = client_state_type;

		const auto client_id = static_cast<client_id_type>(index_in(clients, c));
		const auto mode_id = to_mode_player_id(client_id);

		auto remove_from_mode = [&]() {
			mode_entropy_general cmd;
			cmd.removed_player = mode_id;

			local_collected.control(cmd);
			removed_someone_already = true;
		};

		if (c.pending_entropies.size() > max_pending_commands_for_client_v) {
			disconnect_and_unset(client_id);
		}

		if (!c.is_set()) {
			if (!removed_someone_already) {
				if (character_exists_for(mode_id)) {
					remove_from_mode();
				}
			}

			continue;
		}

		auto contribute_to_step_entropy = [&]() {
#if 0
			c.pending_entropies.set_lower_limit(
				c.settings.net.requested_jitter_buffer_ms * inv_simulation_delta_ms;
			);
#endif

			const auto jitter_vars = c.settings.net.jitter;
			const auto jitter_squash_steps = in_steps(jitter_vars.merge_commands_when_above_ms);

			auto& inputs = c.pending_entropies;

			if (const auto num_pending = inputs.size(); num_pending > 0) {
				const bool should_squash = num_pending >= jitter_squash_steps;

				total_client_entropy entropy;

				c.num_entropies_accepted = [&]() {
					if (should_squash) {
						for (const auto& e : inputs) {
							entropy += e;
						}

						inputs.clear();

						return static_cast<uint8_t>(num_pending);
					}

					entropy = inputs[0];
					inputs.erase(inputs.begin());

					return static_cast<uint8_t>(1);
				}();

				accept_entropy_of_client(mode_id, entropy);
			}
		};

		auto add_client_to_mode = [&]() {
			auto final_nickname = get_arena_handle().on_mode(
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

			if (final_nickname.empty()) {
				return false;
			}

			mode_entropy_general cmd;

			cmd.added_player = add_player_input {
				mode_id,
				std::move(final_nickname),
				faction_type::SPECTATOR
			};

			local_collected.control(cmd);
			added_someone_already = true;

			return true;
		};

		auto kick_if_afk = [&](){
			if (c.should_kick_due_to_inactivity(vars, server_time)) {
				disconnect_and_unset(client_id);
				remove_from_mode();
			}
		};

		auto send_state_for_the_first_time = [&]() {
			const auto sent_client_id = static_cast<uint32_t>(client_id);

			server->send_payload(
				client_id, 
				game_channel_type::SERVER_SOLVABLE_AND_STEPS, 

				vars
			);

			server->send_payload(
				client_id, 
				game_channel_type::SERVER_SOLVABLE_AND_STEPS, 

				buffers,

				initial_arena_state_payload<true> {
					scene.world.get_solvable().significant,
					current_mode,
					sent_client_id
				}
			);
		};

		if (!added_someone_already) {
			if (c.state > client_state_type::PENDING_WELCOME) {
				if (!character_exists_for(mode_id)) {
					if (add_client_to_mode()) {
						if (c.state == S::WELCOME_ARRIVED) {
							send_state_for_the_first_time();

							c.state = S::RECEIVING_INITIAL_STATE;
						}
					}
					else {
						disconnect_and_unset(client_id);
						continue;
					}
				}
			}
		}

		if (!removed_someone_already) {
			kick_if_afk();
		}

#if 0
		else if (c.state == S::RECEIVING_INITIAL_STATE_CORRECTION) {
			if (!server->has_messages_to_send(client_id, game_channel_type::SERVER_SOLVABLE_AND_STEPS)) {
				c.set_in_game(server_time);
			}

		}
#endif
		if (c.state == S::IN_GAME) {
			contribute_to_step_entropy();
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
			LOG("Failed to read payload from the client. Disconnecting.");
			return abort_v;
		}
	}

	using S = client_state_type;
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
			c.state = S::WELCOME_ARRIVED;
		}
	}
	else if constexpr (std::is_same_v<T, total_mode_player_entropy>) {
		if (c.state == S::RECEIVING_INITIAL_STATE) {
			c.set_in_game(server_time);
		}

		if (c.state != S::IN_GAME) {
			/* 
				The client shall not send commands until it receives the first step entropy,
				at which point being IN_GAME is guaranteed.

				Actually, wouldn't the ack for the last fragment come along the first client commands?
			*/

			LOG("Client has sent its command too early (state: %x). Disconnecting.", c.state);

			return abort_v;
		}

		c.pending_entropies.emplace_back(std::move(payload));
		//LOG("Received %x th command from client. ", c.pending_entropies.size());
	}
	else {
		static_assert(always_false_v<T>, "Unhandled payload type.");
	}

	c.last_valid_activity_time = server_time;
	return message_handler_result::CONTINUE;
}

void server_setup::handle_client_messages() {
	auto& message_handler = *this;
	server->advance(server_time, message_handler);
}

void server_setup::send_server_step_entropies(const server_step_entropy& total) {
	for (auto& c : clients) {
		if (!c.is_set()) {
			continue;
		}

		if (c.state < client_state_type::RECEIVING_INITIAL_STATE) {
			continue;
		}

		const auto client_id = static_cast<client_id_type>(index_in(clients, c));
		// const auto mode_id = to_mode_player_id(client_id);

		server_step_entropy_for_client for_client;
		for_client.num_entropies_accepted = c.num_entropies_accepted;
		for_client.total = total;

		c.num_entropies_accepted = 0;

		server->send_payload(
			client_id, 
			game_channel_type::SERVER_SOLVABLE_AND_STEPS,

			for_client
		);
	}
}

void server_setup::reinfer_if_necessary_for(const server_step_entropy& entropy) {
	if (entropy.general.added_player) {
		cosmic::reinfer_solvable(get_arena_handle().get_cosmos());
	}
}

void server_setup::send_packets_if_its_time() {
	auto& ticks_remaining = ticks_until_sending_packets;

	if (ticks_remaining == 0) {
		server->send_packets();

		ticks_remaining = vars.send_updates_once_every_tick;
		--ticks_remaining;
	}
}

double server_setup::get_inv_tickrate() const {
	return get_arena_handle().get_inv_tickrate();
}

double server_setup::get_audiovisual_speed() const {
	return get_arena_handle().get_audiovisual_speed();
}

custom_imgui_result server_setup::perform_custom_imgui(const perform_custom_imgui_input in) {
	if (!server->is_running()) {
		using namespace augs::imgui;

		ImGui::SetNextWindowPosCenter();

		ImGui::SetNextWindowSize((vec2(ImGui::GetIO().DisplaySize) * 0.3f).operator ImVec2(), ImGuiCond_FirstUseEver);

		const auto window_name = "Connection status";
		auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

		const auto addr = yojimbo::Address(last_start.ip.c_str(), last_start.port);

		{
			char buffer[256];
			addr.ToString(buffer, sizeof(buffer));
			text("Failed to host the server at address: %x", buffer);
		}

		text("\n");
		ImGui::Separator();

		if (ImGui::Button("Go back")) {
			return custom_imgui_result::GO_TO_MAIN_MENU;
		}
	}

	return arena_base::perform_custom_imgui(in);
}

setup_escape_result server_setup::escape() {
	if (!is_gameplay_on()) {
		return setup_escape_result::GO_TO_MAIN_MENU;
	}

	return setup_escape_result::IGNORE;
}

bool server_setup::is_gameplay_on() const {
	return server->is_running();
}
