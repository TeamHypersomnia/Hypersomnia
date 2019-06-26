#include "augs/misc/pool/pool_io.hpp"
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

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_file.h"

#include "application/arena/arena_handle.h"
#include "application/arena/choose_arena.h"
#include "application/gui/client/chat_gui.h"
#include "application/network/payload_easily_movable.h"

const auto connected_and_integrated_v = server_setup::for_each_flags { server_setup::for_each_flag::WITH_INTEGRATED, server_setup::for_each_flag::ONLY_CONNECTED };
const auto only_connected_v = server_setup::for_each_flags { server_setup::for_each_flag::ONLY_CONNECTED };

void server_setup::shutdown() {
	if (server->is_running()) {
		LOG("Shutting down the server.");
		server->stop();
	}
}

/* To avoid incomplete type error */
server_setup::~server_setup() {
	shutdown();
}

server_setup::server_setup(
	sol::state& lua,
	const server_start_input& in,
	const server_vars& initial_vars,
	const client_vars& integrated_client_vars,
	const private_server_vars& private_initial_vars,
	const std::optional<augs::dedicated_server_input> dedicated
) : 
	integrated_client_vars(integrated_client_vars),
	lua(lua),
	last_start(in),
	dedicated(dedicated),
	server(std::make_unique<server_adapter>(in)),
	server_time(yojimbo_time())
{
	const bool force = true;
	apply(initial_vars, force);
	apply(private_initial_vars, force);

	if (private_initial_vars.master_rcon_password.empty()) {
		LOG("WARNING! The master rcon password is empty! This means that only the localhost can access the master rcon.");
	}

	if (private_initial_vars.rcon_password.empty()) {
		LOG("WARNING! The rcon password is empty! This means that only the localhost can access the rcon.");
	}

	if (dedicated == std::nullopt) {
		integrated_client.init(server_time);

		if (!integrated_client_vars.avatar_image_path.empty()) {
			auto& png = integrated_client.meta.avatar.png_bytes;

			try {
				png = augs::file_to_bytes(integrated_client_vars.avatar_image_path);

				const auto size = augs::image::get_png_size(png);

				if (size.x > max_avatar_side_v || size.y > max_avatar_side_v) {
					png.clear();
				}
			}
			catch (...) {
				png.clear();
			}
		}

		rebuild_player_meta_viewables = true;
	}
}

template <class F>
void server_setup::for_each_id_and_client(F&& callback, const server_setup::for_each_flags flags) {
	for (auto& c : clients) {
		const auto client_id = static_cast<client_id_type>(index_in(clients, c));

		if (flags[for_each_flag::ONLY_CONNECTED]) {
			if (!c.is_set()) {
				continue;
			}
		}

		callback(client_id, c);
	}

	if (is_integrated()) {
		if (flags[for_each_flag::WITH_INTEGRATED]) {
			callback(static_cast<client_id_type>(get_admin_player_id().value), integrated_client);
		}
	}
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

entity_id server_setup::get_controlled_character_id() const {
	return on_mode_with_input(
		[&](const auto& typed_mode, const auto& in) {
			(void)in;

			const auto local_id = get_local_player_id();
			const auto local_character = typed_mode.lookup(local_id);

			return local_character;
		}
	);
}

void server_setup::log_malicious_client(const client_id_type id) {
	LOG("Malicious client detected. Details:\n%x", describe_client(id));

#if !IS_PRODUCTION_BUILD
	ensure(false && "Client has sent some invalid data.");
#endif
}

std::string server_setup::find_client_nickname(const client_id_type& id) const {
	const auto& c = clients[id];

	if (!c.is_set()) {
		return {};
	}

	std::string nickname = "";

	get_arena_handle().on_mode(
		[&](const auto& mode) {
			if (const auto entry = mode.find(to_mode_player_id(id))) {
				nickname = ": " + entry->chosen_name;
			}
		}
	);

	if (nickname.empty()) {
		nickname = c.settings.chosen_nickname;
	}

	return nickname;
}

std::string server_setup::describe_client(const client_id_type id) const {
	std::string nickname = find_client_nickname(id);

	return typesafe_sprintf(
		"Id: %x\nNickname%x",
		id,
		nickname.empty() ? std::string(" unknown") : std::string(": " + nickname)
	);
}

net_time_t server_setup::get_current_time() {
	return yojimbo_time();
}

void server_setup::customize_for_viewing(config_lua_table& config) const {
#if !IS_PRODUCTION_BUILD
	config.window.name = "Arena server";
#endif

	if (is_gameplay_on()) {
		get_arena_handle().adjust(config.drawing);
	}
}

void server_setup::apply(const private_server_vars& private_new_vars, const bool force) {
	const auto old_vars = private_vars;
	private_vars = private_new_vars;

	(void)force;
}

void server_setup::apply(const server_vars& new_vars, const bool force) {
	const auto old_vars = vars;
	vars = new_vars;

	if (force || old_vars.current_arena != new_vars.current_arena) {
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

			LOG("Arena named \"%x\" was not found on the server!\nLoading the default arena instead.", new_vars.current_arena);

			const auto test_scene_arena = "";
			choose_arena(test_scene_arena);
		}
	}

	if (force || old_vars.network_simulator != new_vars.network_simulator) {
		server->set(new_vars.network_simulator);
	}
}

void server_setup::apply(const config_lua_table& cfg) {
	const bool force = false;
	apply(cfg.server, force);

	integrated_client_vars = cfg.client;
}

void server_setup::choose_arena(const std::string& name) {
	LOG("Choosing arena: %x", name);

	vars.current_arena = name;

	::choose_arena(
		lua,
		get_arena_handle(),
		vars,
		initial_signi
	);

	if (should_have_admin_character()) {
		mode_entropy_general cmd;

		cmd.added_player = add_player_input {
			get_admin_player_id(),
			integrated_client_vars.nickname,
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
	auto& new_client = clients[id];
	new_client.init(server_time);

	LOG("Client %x connected.", id);
}

void server_setup::unset_client(const client_id_type& id) {
	LOG("Client disconnected. Details:\n%x", describe_client(id));
	clients[id].unset();
}

void server_setup::disconnect_and_unset(const client_id_type& id) {
	server->disconnect_client(id);
	unset_client(id);
}

void server_setup::accept_entropy_of_client(
	const mode_player_id mode_id,
	const total_client_entropy& entropy
) {
	if (!entropy.empty()) {
		step_collected += { mode_id, entropy };
	}
}

void server_setup::advance_clients_state() {
	/* Do it only once per tick */
	bool added_someone_already = false;
	bool removed_someone_already = false;

	const auto inv_simulation_delta_ms = 1.0 / (get_inv_tickrate() * 1000.0);

	auto in_steps = [inv_simulation_delta_ms](const auto ms) {
		return static_cast<uint32_t>(ms * inv_simulation_delta_ms);
	};

	auto character_exists_for = [&](const mode_player_id mode_id) {
		return get_arena_handle().on_mode(
			[&](const auto& typed_mode) {
				return typed_mode.find(mode_id);
			}
		);
	};

	auto process_client = [&](const client_id_type client_id, auto& c) {
		using S = client_state_type;
		const auto mode_id = to_mode_player_id(client_id);

		if (c.is_set()) {
			{
				const auto num_commands = c.pending_entropies.size();
				const auto max_commands = vars.max_buffered_client_commands;

				if (num_commands > max_commands) {
					const auto reason = typesafe_sprintf("number of pending commands (%x) exceeded the maximum of %x.", num_commands, max_commands);
					kick(client_id, reason);
				}
			}

			if (!removed_someone_already) {
				if (c.when_kicked != std::nullopt) {
					if (server_time - *c.when_kicked > vars.max_kick_ban_linger_secs) {
						disconnect_and_unset(client_id);
					}
				}
			}
		}

		if (!c.is_set()) {
			if (!removed_someone_already) {
				if (character_exists_for(mode_id)) {
					ensure(!removed_someone_already);

					mode_entropy_general cmd;
					cmd.removed_player = mode_id;

					local_collected.control(cmd);
					removed_someone_already = true;
				}
			}

			return;
		}

		auto contribute_to_step_entropy = [&]() {
#if 0
			c.pending_entropies.set_lower_limit(
				c.settings.net.requested_jitter_buffer_ms * inv_simulation_delta_ms;
			);
#endif

			const auto jitter_vars = c.settings.net.jitter;
			const auto jitter_squash_steps = std::max(jitter_vars.buffer_at_least_steps, in_steps(jitter_vars.buffer_at_least_ms));

			auto& inputs = c.pending_entropies;

			if (const auto num_pending = inputs.size(); num_pending > 0) {
				const bool should_squash = num_pending >= jitter_squash_steps;

				total_client_entropy entropy;

				c.num_entropies_accepted = [&]() {
					if (should_squash) {
						const auto num_squashed = static_cast<uint8_t>(
							std::min(
								num_pending,
								static_cast<std::size_t>(c.settings.net.jitter.max_commands_to_squash_at_once)
							)
						);

						for (std::size_t i = 0; i < num_squashed; ++i) {
							entropy += inputs[i];
						}

						if (num_squashed == num_pending) {
							inputs.clear();
						}
						else {
							erase_first_n(inputs, num_squashed);
						}

						return static_cast<uint8_t>(num_squashed);
					}

					entropy = inputs[0];
					erase_first_n(inputs, 1);

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
				kick(client_id, "No messages arrived for too long!");
			}

			if (c.should_kick_due_to_afk(vars, server_time)) {
				kick(client_id, "AFK!");
			}
		};

		auto send_state_for_the_first_time = [&]() {
			const auto sent_client_id = static_cast<uint32_t>(client_id);

			server->send_payload(
				client_id, 
				game_channel_type::SERVER_SOLVABLE_AND_STEPS, 

				vars
			);

			const auto rcon_level = get_rcon_level(client_id);

			server->send_payload(
				client_id, 
				game_channel_type::SERVER_SOLVABLE_AND_STEPS, 

				buffers,

				scene.world.get_common_significant().flavours,

				initial_arena_state_payload<true> {
					scene.world.get_solvable().significant,
					current_mode,
					sent_client_id,
					rcon_level
				}
			);

			auto broadcast_avatar = [this, recipient_client_id = client_id](const auto client_id_of_avatar, auto& cc) {
				server->send_payload(
					recipient_client_id,
					game_channel_type::COMMUNICATIONS,

					client_id_of_avatar,
					cc.meta.avatar
				);
			};

			for_each_id_and_client(broadcast_avatar, connected_and_integrated_v);

			LOG("Sending initial payload for %x at step: %x", client_id, scene.world.get_total_steps_passed());
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
						return;
					}
				}
			}
		}

		if (c.state == client_state_type::IN_GAME) {
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
	};

	for_each_id_and_client(process_client);
}

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
			LOG("Client %x requested nickname: %x", client_id, std::string(c.settings.chosen_nickname));
			c.state = S::WELCOME_ARRIVED;
		}

		c.last_keyboard_activity_time = server_time;
	}
	else if constexpr (std::is_same_v<T, rcon_command_variant>) {
		const auto level = get_rcon_level(client_id);

		LOG("Detected rcon level: %x", static_cast<int>(level));

		if (level != rcon_level::NONE) {
			const auto result = std::visit(
				[&](const auto& typed_payload) {
					using P = remove_cref<decltype(typed_payload)>;
					using namespace rcon_commands;

					if constexpr(std::is_same_v<P, special>) {
						switch (typed_payload) {
							case special::SHUTDOWN:
							LOG("Shutting down due to rcon's request.");
							schedule_shutdown = true;

							return abort_v;
							break;

							default:

							LOG("Unsupported rcon command.");
							return message_handler_result::CONTINUE;
							break;
						}
					}
					else if constexpr(std::is_same_v<P, std::monostate>) {
						return abort_v;
					}
					else {
						static_assert(always_false_v<P>, "Unhandled rcon command type!");
						return abort_v;
					}
				},
				payload
			);

			if (result == abort_v) {
				return result;
			}

			c.last_keyboard_activity_time = server_time;
		}
		else {
			++c.unauthorized_rcon_commands;

			if (c.unauthorized_rcon_commands > vars.max_unauthorized_rcon_commands) {
				LOG("unauthorized_rcon_commands exceeded max_unauthorized_rcon_commands (%x). Kicking client.", vars.max_unauthorized_rcon_commands);

				return abort_v;
			}
		}
	}
	else if constexpr (std::is_same_v<T, client_requested_chat>) {
		if (c.state == S::IN_GAME) {
			server_broadcasted_chat message;

			message.author = to_mode_player_id(client_id);
			message.message = std::string(payload.message);
			message.target = payload.target;

			broadcast(message);

			c.last_keyboard_activity_time = server_time;
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

		if (!payload.empty()) {
			c.last_keyboard_activity_time = server_time;
		}

		c.pending_entropies.emplace_back(std::move(payload));
		//LOG("Received %x th command from client. ", c.pending_entropies.size());
	}
	else if constexpr (std::is_same_v<T, special_client_request>) {
		switch (payload) {
			case special_client_request::RESYNC:
				if (server_time >= c.last_resync_counter_reset_at + vars.reset_resync_timer_once_every_secs) {
					c.resyncs_counter = 0;
					LOG("Resetting the resync counter.");
				}

				c.last_resync_counter_reset_at = server_time;
				++c.resyncs_counter;

				LOG("Client has asked for a resync no %x.", c.resyncs_counter);

				if (c.resyncs_counter > vars.max_client_resyncs) {
					LOG("Client is asking for a resync too often! Kicking.");
					return abort_v;
				}

				{
					const auto rcon_level = get_rcon_level(client_id);

					server->send_payload(
						client_id, 
						game_channel_type::SERVER_SOLVABLE_AND_STEPS, 

						buffers,

						scene.world.get_common_significant().flavours,

						initial_arena_state_payload<true> {
							scene.world.get_solvable().significant,
							current_mode,
							client_id,
							rcon_level
						}
					);
				}

				reinference_necessary = true;

				break;

			default: return abort_v;
		}
	}
	else if constexpr (std::is_same_v<T, arena_player_avatar_payload>) {
		{
			uint32_t dummy_id = 0;
			arena_player_avatar_payload payload;

			if (read_payload(dummy_id, payload)) {
				try {
					const auto size = augs::image::get_png_size(payload.png_bytes);

					if (size.x > max_avatar_side_v || size.y > max_avatar_side_v) {
						const auto disconnect_reason = typesafe_sprintf("sending an avatar of size %xx%x", size.x, size.y);
						kick(client_id, disconnect_reason);
					}
					else {
						c.meta.avatar = std::move(payload);

						auto update_avatar = [this, client_id_of_avatar = client_id, &client_with_updated_avatar = c](const auto recipient_client_id, auto&) {
							server->send_payload(
								recipient_client_id,
								game_channel_type::COMMUNICATIONS,

								client_id_of_avatar,
								client_with_updated_avatar.meta.avatar
							);
						};

						for_each_id_and_client(update_avatar, only_connected_v);
						rebuild_player_meta_viewables = true;
					}
				}
				catch (...) {
					kick(client_id, "sending an invalid avatar");
				}
			}
			else {
				kick(client_id, "sending an invalid avatar");
			}
		}
	}
	else {
		static_assert(always_false_v<T>, "Unhandled payload type.");
	}

	c.last_valid_message_time = server_time;
	return message_handler_result::CONTINUE;
}

void server_setup::handle_client_messages() {
	auto& message_handler = *this;
	server->advance(server_time, message_handler);
}

void server_setup::send_server_step_entropies(const compact_server_step_entropy& total_input) {
	networked_server_step_entropy total;
	total.payload = total_input;
	total.meta.reinference_necessary = reinference_necessary;
	total.meta.state_hash = [&]() -> decltype(total.meta.state_hash) {
		auto& ticks_remaining = ticks_until_sending_hash;

		if (ticks_remaining == 0) {
			ticks_remaining = vars.state_hash_once_every_tick;
			--ticks_remaining;

			const auto calculated_hash = get_arena_handle().get_cosmos().calculate_solvable_signi_hash<uint32_t>();
			return calculated_hash;
		}

		return std::nullopt;
	}();

	auto process_client = [&](const auto client_id, auto& c) {
		const bool its_time_already = 
			c.state >= client_state_type::RECEIVING_INITIAL_STATE
		;

		if (!its_time_already) {
			return;
		}

		{
			{
				prestep_client_context context;
				context.num_entropies_accepted = c.num_entropies_accepted;

#if CONTEXTS_SEPARATE
				server->send_payload(
					client_id, 
					game_channel_type::SERVER_SOLVABLE_AND_STEPS,

					context
				);
#else
				total.context = context;
#endif
			}

			/* Reset the counter */
			c.num_entropies_accepted = 0;
		}

		/* TODO PERFORMANCE: only serialize the message once and multicast the same buffer to all clients! */
		server->send_payload(
			client_id,
			game_channel_type::SERVER_SOLVABLE_AND_STEPS,

			total
		);
	};

	for_each_id_and_client(process_client, only_connected_v);

	{
		if (server_time - when_last_sent_net_statistics > vars.send_net_statistics_update_once_every_secs) {
			net_statistics_update update;

			auto gather_stats = [&](const auto client_id, auto&) {
				const auto max_ping = std::numeric_limits<uint8_t>::max();
				const auto info = server->get_network_info(client_id);
				const auto rounded_ping = static_cast<int>(std::round(info.rtt_ms));

				const auto clamped_ping = std::clamp(rounded_ping, 0, int(max_ping));

				last_player_metas[client_id].stats.ping = clamped_ping;

				update.ping_values.push_back(static_cast<uint8_t>(clamped_ping));
			};

			auto send_stats  = [&](const auto client_id, auto&) {
				server->send_payload(
					client_id,
					game_channel_type::VOLATILE_STATISTICS,

					update
				);
			};

			for_each_id_and_client(gather_stats, only_connected_v);
			for_each_id_and_client(send_stats, only_connected_v);

			when_last_sent_net_statistics = server_time;
		}
	}
}

void server_setup::reinfer_if_necessary_for(const compact_server_step_entropy& entropy) {
	if (reinference_necessary || logically_set(entropy.general.added_player)) {
		LOG("Server: Added player or reinference_necessary. Will reinfer to sync.");
		cosmic::reinfer_solvable(get_arena_handle().get_cosmos());
		reinference_necessary = false;
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
	else {
		if (is_integrated()) {
			auto& chat = integrated_client_gui.chat;

			if (chat.perform_input_bar(integrated_client_vars.client_chat)) {
				server_broadcasted_chat message;

				message.author = get_admin_player_id();
				message.message = std::string(chat.current_message);
				message.target = chat.target;

				broadcast(message);

				chat.current_message.clear();
			}
		}
	}

	return arena_base::perform_custom_imgui(in);
}

setup_escape_result server_setup::escape() {
	if (!is_gameplay_on()) {
		return setup_escape_result::GO_TO_MAIN_MENU;
	}

	return arena_base::escape();
}

bool server_setup::is_gameplay_on() const {
	return is_running();
}

bool server_setup::is_running() const {
	return server->is_running();
}

bool server_setup::should_have_admin_character() const {
	return dedicated == std::nullopt;
}

void server_setup::sleep_until_next_tick() {
	return;
	const auto sleep_dt = get_current_time() - server_time;

	if (sleep_dt > 0.0) {
		yojimbo_sleep(sleep_dt * 0.9);
	}
}

void server_setup::update_stats(server_network_info& info) const {
	info = server->get_server_network_info();
}

server_step_entropy server_setup::unpack(const compact_server_step_entropy& n) const {
	return n.unpack(
		[&](const mode_player_id& mode_id) {
			return get_arena_handle().on_mode(
				[&](const auto& typed_mode) {
					return typed_mode.lookup(mode_id);
				}
			);
		}
	);
}

augs::path_type server_setup::get_unofficial_content_dir() const {
	const auto& name = vars.current_arena;

	if (name.empty()) {
		return {};
	}

	const auto paths = arena_paths(name);
	return paths.folder_path;
}

bool safe_equal(const decltype(requested_client_settings::rcon_password)& candidate_password, const std::string& actual_password) {
	const bool rcon_is_disabled = actual_password.empty();

	if (rcon_is_disabled) {
		return false;
	}

	int matches = 0;

	for (std::size_t i = 0; i < std::min(static_cast<std::size_t>(candidate_password.size()), actual_password.size()); ++i) {
		if (candidate_password.data()[i] == actual_password[i]) {
			matches++;
		}
		else {
			matches--;
		}
	}

	return matches == static_cast<int>(actual_password.size());
}

rcon_level server_setup::get_rcon_level(const client_id_type& id) const { 
	const auto& c = clients[id];

	if (vars.auto_authorize_loopback_for_rcon) {
		if (server->get_client_address(id).IsLoopback()) {
			LOG("Auto-authorizing the loopback client for master rcon.");
			return rcon_level::MASTER;
		}
	}

	if (::safe_equal(c.settings.rcon_password, private_vars.master_rcon_password)) {
		LOG("Authorized the remote client for master rcon.");
		return rcon_level::MASTER;
	}

	if (::safe_equal(c.settings.rcon_password, private_vars.rcon_password)) {
		LOG("Authorized the remote client for basic rcon.");
		return rcon_level::BASIC;
	}

	LOG("RCON disabled for this client.");

	return rcon_level::NONE;
}

void server_setup::broadcast(const ::server_broadcasted_chat& payload) {
	const auto sender_player = get_arena_handle().on_mode(
		[&](const auto& typed_mode) {
			return typed_mode.find(payload.author);
		}
	);

	std::string sender_player_nickname;
	auto sender_player_faction = faction_type::SPECTATOR;

	if (sender_player != nullptr) {
		sender_player_faction = sender_player->faction;
		sender_player_nickname = sender_player->chosen_name;
	}

	bool integrated_received = false;

	const auto new_entry = chat_gui_entry::from(
		payload,
		get_current_time(),
		sender_player_nickname,
		sender_player_faction
	);

	auto send_it = [&](const auto recipient_client_id, auto&) {
		if (payload.target == chat_target_type::TEAM_ONLY) {
			const auto recipient_player = get_arena_handle().on_mode(
				[&](const auto& typed_mode) {
					return typed_mode.find(to_mode_player_id(recipient_client_id));
				}
			);

			const auto recipient_player_faction = recipient_player ? recipient_player->faction : faction_type::SPECTATOR;

			if (sender_player_faction != recipient_player_faction) {
				return;
			}
		}

		if (to_mode_player_id(recipient_client_id) == get_admin_player_id()) {
			integrated_received = true;
		}
		else {
			server->send_payload(
				recipient_client_id,
				game_channel_type::COMMUNICATIONS,

				payload
			);
		}
	};

	for_each_id_and_client(send_it, connected_and_integrated_v);

	if (integrated_received) {
		integrated_client_gui.chat.add_entry(std::move(new_entry));
	}

	if (is_dedicated() || integrated_received) {
		LOG(new_entry.operator std::string());
	}
}

message_handler_result server_setup::abort_or_kick_if_debug(const client_id_type& id, const std::string& reason) {
#if IS_PRODUCTION_BUILD
	LOG(find_client_nickname(id) + " was forcefully disconnected the server. Reason: %x", reason);
	return message_handler_result::ABORT_AND_DISCONNECT;
#else
	kick(id, reason);
	return message_handler_result::CONTINUE;
#endif
}

void server_setup::kick(const client_id_type& id, const std::string& reason) {
	auto& c = clients[id];

	if (!c.is_set()) {
		return;
	}

	if (c.when_kicked != std::nullopt) {
		return;
	}

	c.when_kicked = server_time;

	server_broadcasted_chat message;
	message.message = reason;
	message.target = chat_target_type::KICK;
	message.author = to_mode_player_id(id);

	broadcast(message);
}

void server_setup::ban(const client_id_type& id, const std::string& reason) {
	auto& c = clients[id];

	if (!c.is_set()) {
		return;
	}

	if (c.when_kicked == std::nullopt) {
		c.when_kicked = server_time;
	}

	server_broadcasted_chat message;
	message.message = reason;
	message.target = chat_target_type::BAN;
	message.author = to_mode_player_id(id);

	broadcast(message);
}

std::optional<arena_player_metas> server_setup::get_new_player_metas() {
	if (rebuild_player_meta_viewables) {
		auto& metas = last_player_metas;
		
		auto make_meta = [&](const auto client_id, const auto& cc) {
			metas[client_id].avatar.png_bytes = cc.meta.avatar.png_bytes;
		};

		for_each_id_and_client(make_meta, connected_and_integrated_v);

		rebuild_player_meta_viewables = false;
		return metas;
	}

	return std::nullopt;
}

const arena_player_metas* server_setup::find_player_metas() const {
	return std::addressof(last_player_metas);
}
	
bool server_setup::handle_input_before_game(
	const handle_input_before_game_input in
) {
	ensure(is_integrated());

	if (arena_base::handle_input_before_game(in)) {
		return true;
	}

	if (integrated_client_gui.control(in)) {
		return true;
	}

	return false;
}

void server_setup::draw_custom_gui(const draw_setup_gui_input& in) const {
	ensure(is_integrated());

	using namespace augs::gui::text;

	integrated_client_gui.chat.draw_recent_messages(
		in.drawer,
		integrated_client_vars.client_chat,
		in.config.faction_view,
		in.gui_fonts.gui,
		get_current_time()
	);

	arena_base::draw_custom_gui(in);
}

bool server_setup::is_integrated() const {
	return dedicated == std::nullopt;
}

bool server_setup::is_dedicated() const {
	return dedicated != std::nullopt;
}

#include "augs/readwrite/to_bytes.h"

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>
#include "augs/misc/lua/lua_utils.h"
#include <sol2/sol.hpp>
#include "augs/readwrite/lua_file.h"

TEST_CASE("NetSerialization EmptyEntropies") {
	{
		net_messages::server_step_entropy ss;
		ss.Release();

		{
			REQUIRE(ss.bytes.size() == 0);

			networked_server_step_entropy sent;
			ss.write_payload(sent);

			/* One byte for num of entropies accepted, second byte for signifying empty entropy */
			REQUIRE(ss.bytes.size() == 2);
		}
	}

	{
		net_messages::server_step_entropy ss;
		ss.Release();

		{
			REQUIRE(ss.bytes.size() == 0);

			networked_server_step_entropy sent;
			sent.meta.state_hash = 0xdeadbeef;
			ss.write_payload(sent);

			/* One byte for num of entropies accepted, second byte for signifying empty entropy and existent hash, additional bytes for hash */
			REQUIRE(ss.bytes.size() == 2 + sizeof(decltype(sent.meta.state_hash)::value_type));
		}
	}

	{
		net_messages::client_entropy ss;
		ss.Release();

		{
			REQUIRE(ss.bytes.size() == 0);

			total_client_entropy sent;
			ss.write_payload(sent);

			REQUIRE(ss.bytes.size() == 1);
		}
	}
}

TEST_CASE("NetSerialization ServerEntropy") {
	net_messages::server_step_entropy ss;
	ss.Release();
	
	networked_server_step_entropy sent;

	const auto naive_bytes = [&]() {
		total_mode_player_entropy t;
		t.mode = mode_commands::team_choice { faction_type::METROPOLIS };
		t.mode = mode_commands::spell_purchase { spell_id::of<fury_of_the_aeons>() };

		total_mode_player_entropy tt;
		tt.cosmic.cast_spell.set<ultimate_wrath_of_the_aeons>();
		tt.cosmic.motions[game_motion_type::MOVE_CROSSHAIR] = { 342, 432534 };
		tt.cosmic.intents.push_back({ game_intent_type::USE, intent_change::PRESSED });
		tt.cosmic.intents.push_back({ game_intent_type::MOVE_FORWARD, intent_change::RELEASED });

		auto second = mode_player_id::first();
		second.value++;

		auto third = second;
		third.value++;

		sent.payload.players.push_back({ mode_player_id::machine_admin(), t });
		sent.payload.players.push_back({ second, {} });
		sent.payload.players.push_back({ third, tt });
		sent.payload.players.push_back({ mode_player_id::first(), {} });

		REQUIRE(ss.write_payload(sent));

		return augs::to_bytes(sent);	
	}();

	networked_server_step_entropy received;
	const auto naively_received = augs::from_bytes<networked_server_step_entropy>(naive_bytes);

	REQUIRE(ss.read_payload(received));

	if (!(received == sent) || !(received == naively_received)) {
		auto lua = augs::create_lua_state();

		augs::save_as_lua_table(lua, sent, "sent.lua");
		augs::save_as_lua_table(lua, received, "received.lua");
	}

	REQUIRE(received == naively_received);
	REQUIRE(received == sent);

	const auto naive_bytes_of_received = augs::to_bytes(received);
	REQUIRE(naive_bytes_of_received == naive_bytes);
}

TEST_CASE("NetSerialization ServerEntropySecond") {
	net_messages::server_step_entropy ss;
	ss.Release();

	networked_server_step_entropy sent;
	sent.meta.state_hash = 0xdeadbeef;
	sent.meta.reinference_necessary = true;

	const auto naive_bytes = [&]() {
		total_mode_player_entropy t;

		item_flavour_id bought_item;
		bought_item.type_id.set<shootable_weapon>();
		bought_item.raw.indirection_index = 11;
		bought_item.raw.version  = 11;

		t.mode = bought_item;

		total_mode_player_entropy tt;
		tt.cosmic.cast_spell.set<ultimate_wrath_of_the_aeons>();
		tt.cosmic.motions[game_motion_type::MOVE_CROSSHAIR] = { 34, 111 };
		tt.cosmic.intents.push_back({ game_intent_type::DROP, intent_change::RELEASED });
		tt.cosmic.intents.push_back({ game_intent_type::DROP, intent_change::RELEASED });
		tt.cosmic.intents.push_back({ game_intent_type::DROP, intent_change::RELEASED });

		auto second = mode_player_id::first();
		second.value++;

		auto third = second;
		third.value++;

		sent.payload.players.push_back({ mode_player_id::machine_admin(), {} });
		sent.payload.players.push_back({ second, tt });
		sent.payload.players.push_back({ mode_player_id::first(), tt });
		sent.payload.players.push_back({ third, {} });

		sent.payload.general.added_player.id = mode_player_id::machine_admin();
		sent.payload.general.added_player.name = "proplayerrrrrrrrrr";
		sent.payload.general.added_player.faction = faction_type::DEFAULT;
		sent.payload.general.removed_player = third;
		sent.payload.general.special_command.emplace<mode_restart_command>();

		REQUIRE(ss.write_payload(sent));

		return augs::to_bytes(sent);	
	}();

	networked_server_step_entropy received;
	const auto naively_received = augs::from_bytes<networked_server_step_entropy>(naive_bytes);

	REQUIRE(ss.read_payload(received));

	if (!(received == sent) || !(received == naively_received)) {
		auto lua = augs::create_lua_state();

		augs::save_as_lua_table(lua, sent, "sent.lua");
		augs::save_as_lua_table(lua, received, "received.lua");
	}

	REQUIRE(received == naively_received);
	REQUIRE(received == sent);
}

TEST_CASE("NetSerialization ClientEntropy") {
	net_messages::client_entropy ss;
	ss.Release();

	total_client_entropy sent;

	const auto naive_bytes = [&]() {
		sent.mode = mode_commands::team_choice { faction_type::RESISTANCE };
		sent.mode = mode_commands::spell_purchase { spell_id::of<haste>() };

		sent.cosmic.cast_spell.set<ultimate_wrath_of_the_aeons>();
		sent.cosmic.motions[game_motion_type::MOVE_CROSSHAIR] = { 342, 432534 };
		sent.cosmic.intents.push_back({ game_intent_type::USE, intent_change::PRESSED });
		sent.cosmic.intents.push_back({ game_intent_type::MOVE_FORWARD, intent_change::RELEASED });

		ss.write_payload(sent);

		return augs::to_bytes(sent);	
	}();

	total_client_entropy received;
	const auto naively_received = augs::from_bytes<total_client_entropy>(naive_bytes);

	REQUIRE(ss.read_payload(received));

	if (!(received == sent) || !(received == naively_received)) {
		auto lua = augs::create_lua_state();

		augs::save_as_lua_table(lua, sent, "sent.lua");
		augs::save_as_lua_table(lua, received, "received.lua");
	}

	REQUIRE(received == naively_received);
	REQUIRE(received == sent);

	const auto naive_bytes_of_received = augs::to_bytes(received);
	REQUIRE(naive_bytes_of_received == naive_bytes);
}

#endif
