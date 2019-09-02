#include "augs/misc/pool/pool_io.hpp"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/client/client_setup.h"
#include "application/config_lua_table.h"

#include "application/network/client_adapter.hpp"
#include "application/network/net_message_translation.h"

#include "game/cosmos/change_solvable_significant.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"

#include "application/arena/choose_arena.h"

#include "augs/filesystem/file.h"

#include "augs/templates/introspection_utils/introspective_equal.h"
#include "augs/gui/text/printer.h"
#include "augs/readwrite/byte_file.h"
#include "application/network/payload_easily_movable.h"
#include "augs/misc/readable_bytesize.h"
#include "game/cosmos/for_each_entity.h"
#include "game/cosmos/entity_type_traits.h"

#include "application/gui/config_nvp.h"
#include "application/gui/do_server_vars.h"

#include "application/setups/client/rcon_pane.h"
#include "application/gui/pretty_tabs.h"

void snap_interpolated_to_logical(cosmos&);

client_setup::client_setup(
	sol::state& lua,
	const client_start_input& in,
	const client_vars& initial_vars
) : 
	lua(lua),
	last_start(in.get_address_and_port()),
	vars(initial_vars),
	adapter(std::make_unique<client_adapter>()),
	client_time(get_current_time()),
	when_initiated_connection(get_current_time())
{
	LOG("Initializing connection with %x", last_start.connect_address);

	if (!nickname_len_in_range(vars.nickname.length())) {
		const auto reason = typesafe_sprintf(
			"The nickname should be between %x and %x bytes.", 
			min_nickname_length_v,
			max_nickname_length_v
		);

		set_disconnect_reason(reason);
	}
	else {
		augs::network::enable_detailed_logs(true);

		const auto resolution = adapter->connect(last_start);

		if (resolution.result == resolve_result_type::COULDNT_RESOLVE_HOST) {
			const auto reason = typesafe_sprintf(
				"Couldn't resolve host: %x", 
				resolution.host
			);

			set_disconnect_reason(reason);
		}
		else if (resolution.result == resolve_result_type::INVALID_ADDRESS) {
			const auto reason = typesafe_sprintf(
				"The address: \"%x\" is invalid!", last_start.connect_address
			);

			set_disconnect_reason(reason);
		}
		else {
			ensure_eq(resolution.result, resolve_result_type::OK);
		}
	}
}

client_setup::~client_setup() {
	LOG("Client setup dtor");
	disconnect();

	augs::network::enable_detailed_logs(false);
}

net_time_t client_setup::get_current_time() {
	return yojimbo_time();
}

entity_id client_setup::get_controlled_character_id() const {
	if (!is_gameplay_on()) {
		return entity_id::dead();
	}

	return on_mode_with_input(
		[&](const auto& typed_mode, const auto& in) {
			(void)in;

			const auto local_id = get_local_player_id();
			const auto local_character = typed_mode.lookup(local_id);

			return local_character;
		}
	);
}

void client_setup::customize_for_viewing(config_lua_table& config) const {
#if !IS_PRODUCTION_BUILD
	config.window.name = "Arena client";
#endif

	if (is_gameplay_on()) {
		get_arena_handle(client_arena_type::REFERENTIAL).adjust(config.drawing);
	}
}

void client_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

bool client_setup::is_spectating_referential() const {
	return vars.spectate_referential_state && arena_gui.spectator.has_been_drawn;
}

client_arena_type client_setup::get_viewed_arena_type() const {
	if (is_spectating_referential()) {
		return client_arena_type::REFERENTIAL;
	}

#if USE_CLIENT_PREDICTION
	return client_arena_type::PREDICTED;
#else
	return client_arena_type::REFERENTIAL;
#endif

}

online_arena_handle<false> client_setup::get_arena_handle(std::optional<client_arena_type> c) {
	if (c == std::nullopt) {
		c = get_viewed_arena_type();
	}

	return get_arena_handle_impl<online_arena_handle<false>>(*this, *c);
}

online_arena_handle<true> client_setup::get_arena_handle(std::optional<client_arena_type> c) const {
	if (c == std::nullopt) {
		c = get_viewed_arena_type();
	}

	return get_arena_handle_impl<online_arena_handle<true>>(*this, *c);
}

double client_setup::get_inv_tickrate() const {
	if (!is_gameplay_on()) {
		return default_inv_tickrate;
	}

	return get_arena_handle().get_inv_tickrate();
}

double client_setup::get_audiovisual_speed() const {
	if (!is_gameplay_on()) {
		return 1.0;
	}

	return get_arena_handle().get_audiovisual_speed();
}

using initial_payload = initial_arena_state_payload<false>;

template <class T, class F>
message_handler_result client_setup::handle_server_message(
	F&& read_payload
) {
	constexpr auto abort_v = message_handler_result::ABORT_AND_DISCONNECT;
	constexpr auto continue_v = message_handler_result::CONTINUE;
	constexpr bool is_easy_v = payload_easily_movable_v<T>;

	std::conditional_t<is_easy_v, T, std::monostate> payload;

	if constexpr(is_easy_v) {
		if (!read_payload(payload)) {
			return abort_v;
		}
	}

	if constexpr (std::is_same_v<T, server_solvable_vars>) {
		const bool are_initial_vars = state == client_state_type::PENDING_WELCOME;
		const auto& new_vars = payload;

		if (are_initial_vars) {
			LOG("Received initial vars from the server");
			state = client_state_type::RECEIVING_INITIAL_STATE;
		}
		else {
			LOG("Received corrected vars from the server");
		}

		const auto& new_arena = new_vars.current_arena;

		LOG_NVPS(new_arena);

		if (are_initial_vars || new_arena != sv_solvable_vars.current_arena) {
			LOG("Client loads arena: %x", new_arena);

			try {
				::choose_arena(
					lua,
					get_arena_handle(client_arena_type::REFERENTIAL),
					new_vars,
					initial_signi
				);

				arena_gui.reset();
				client_gui.rcon = {};
			}
			catch (const augs::file_open_error& err) {
				set_disconnect_reason(typesafe_sprintf(
					"Failed to load arena: \"%x\".\n"
					"The arena files might be corrupt, or they might be missing.\n"
					"Please check if \"%x\" folder resides within \"%x\" directory.\n"
					"\nDetails: \n%x",
					new_arena,
					new_arena,
					"arenas",
					err.what()
				));

				return abort_v;
			}

			/* Prepare the predicted cosmos. */
			predicted_cosmos = scene.world;
		}

		sv_solvable_vars = new_vars;
		last_applied_sv_solvable_vars = new_vars;
		edited_sv_solvable_vars = new_vars;

		applying_sv_solvable_vars = false;
	}
	else if constexpr (std::is_same_v<T, server_vars>) {
		const auto& new_vars = payload;

		sv_vars = new_vars;
		last_applied_sv_vars = new_vars;
		edited_sv_vars = new_vars;

		applying_sv_vars = false;
	}
	else if constexpr (std::is_same_v<T, server_broadcasted_chat>) {
		const auto author_id = payload.author;

		const auto sender_player = get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
			[&](const auto& typed_mode) {
				return typed_mode.find(author_id);
			}
		);

		std::string sender_player_nickname;
		auto sender_player_faction = faction_type::SPECTATOR;

		if (sender_player != nullptr) {
			sender_player_faction = sender_player->faction;
			sender_player_nickname = sender_player->chosen_name;
		}
		else {
			if (!author_id.is_set()) {
				sender_player_nickname = "Client";
			}
			else {
				sender_player_nickname = "Unknown player";
			}
		}

		{
			auto new_entry = chat_gui_entry::from(
				payload,
				get_current_time(),
				sender_player_nickname,
				sender_player_faction
			);

			LOG(new_entry.operator std::string());
			client_gui.chat.add_entry(std::move(new_entry));
		}

		if (payload.recipient_shall_kindly_leave) {
			if (payload.target == chat_target_type::SERVER_SHUTTING_DOWN) {
				const auto msg = std::string(payload.message);

				std::string reason_str;

				if (msg.size() > 0) {
					reason_str = "\n\n" + reason_str;
				}

				set_disconnect_reason(typesafe_sprintf(
					"The server is shutting down.%x", 
					reason_str
				), true);
			}
			else {
				std::string kicked_or_banned;

				if (payload.target == chat_target_type::KICK) {
					kicked_or_banned = "kicked";
				}
				else if (payload.target == chat_target_type::BAN) {
					kicked_or_banned = "banned";
				}

				set_disconnect_reason(typesafe_sprintf(
					"You were %x from the server.\nReason: %x", 
					kicked_or_banned,
					std::string(payload.message)
				), true);
			}

			LOG_NVPS(last_disconnect_reason);

			return abort_v;
		}	
	}
	else if constexpr (std::is_same_v<T, initial_payload>) {
		if (!now_resyncing && state != client_state_type::RECEIVING_INITIAL_STATE) {
			LOG("The server has sent initial state early (state: %x). Disconnecting.", state);
			log_malicious_server();
			return abort_v;
		}

		const bool was_resyncing = now_resyncing;

		now_resyncing = false;

		uint32_t read_client_id;

		cosmic::change_solvable_significant(
			scene.world, 
			[&](cosmos_solvable_significant& signi) {
				read_payload(
					buffers,

					initial_signi,

					initial_payload {
						signi,
						current_mode,
						read_client_id,
						rcon
					}
				);

				return changer_callback_result::REFRESH;
			}
		);

		client_player_id = static_cast<mode_player_id>(read_client_id);

		LOG("Received initial state from the server at step: %x.", scene.world.get_timestamp().step);
		LOG("Received client id: %x", client_player_id.value);

		state = client_state_type::IN_GAME;

		auto predicted = get_arena_handle(client_arena_type::PREDICTED);
		const auto referential = get_arena_handle(client_arena_type::REFERENTIAL);

		auto& predicted_cosmos = predicted.advanced_cosm;

		if (was_resyncing) {
			::save_interpolations(receiver.transfer_caches, std::as_const(predicted_cosmos));
		}

		predicted.transfer_all_solvables(referential);

		if (was_resyncing) {
			::restore_interpolations(receiver.transfer_caches, predicted_cosmos);
			receiver.schedule_reprediction = true;
		}

		receiver.clear_incoming();

		if (!was_resyncing) {
			snap_interpolated_to_logical(predicted_cosmos);
		}
	}
#if CONTEXTS_SEPARATE
	else if constexpr (std::is_same_v<T, prestep_client_context>) {
		if (state != client_state_type::IN_GAME) {
			LOG("The server has sent prestep context too early (state: %x). Disconnecting.", state);

			log_malicious_server();
			return abort_v;
		}

		receiver.acquire_next_server_entropy(payload);
	}
#endif
	else if constexpr (std::is_same_v<T, networked_server_step_entropy>) {
		if (state != client_state_type::IN_GAME) {
			LOG("The server has sent entropy too early (state: %x). Disconnecting.", state);

			log_malicious_server();
			return abort_v;
		}

		receiver.acquire_next_server_entropy(
			payload.context,
			payload.meta, 
			payload.payload
		);

		const auto& max_commands = vars.max_buffered_server_commands;
		const auto num_commands = receiver.incoming_entropies.size();

		if (num_commands > max_commands) {
			set_disconnect_reason(typesafe_sprintf(
				"Number of buffered server commands (%x) exceeded max_buffered_server_commands (%x).", 
				num_commands,
				max_commands
			));

			LOG_NVPS(last_disconnect_reason);

			return abort_v;
		}

		//LOG("Received %x th entropy from the server", receiver.incoming_entropies.size());
		//LOG_NVPS(payload.num_entropies_accepted);
	}
	else if constexpr (std::is_same_v<T, public_settings_update>) {
		/* 
			We can assign it right away and it won't desync,
			because it only affects the incoming entropies and they are unpacked on the go
			whenever networked_server_step_entropy arrives.

			networked_server_step_entropy and public_settings_update are on the same channel.
		*/

		player_metas[payload.subject_id.value].public_settings = payload.new_settings;
	}
	else if constexpr (std::is_same_v<T, net_statistics_update>) {
		const auto& ping_values = payload.ping_values;

		int ping_value_i = 0;

		get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
			[&](const auto& typed_mode) {
				typed_mode.for_each_player_id(
					[&](const mode_player_id& id) {
						if (ping_value_i < static_cast<int>(ping_values.size())) {
							player_metas[id.value].stats.ping = ping_values[ping_value_i++];

							return callback_result::CONTINUE;
						}

						return callback_result::ABORT;
					}
				);
			}
		);

	}
	else if constexpr (std::is_same_v<T, arena_player_avatar_payload>) {
		session_id_type session_id;
		arena_player_avatar_payload new_avatar;

		const bool result = read_payload(
			session_id,
			new_avatar
		);

		if (!result) {
			return abort_v;
		}

		auto p = untimely_payload { session_id, std::move(new_avatar) };

		if (!push_or_handle(p)) {
			return abort_v;
		}
	}
	else {
		static_assert(always_false_v<T>, "Unhandled payload type.");
	}

	return continue_v;
}

bool client_setup::push_or_handle(untimely_payload& p) {
	const auto session_id = p.associated_id;

	if (logically_set(find_by(session_id))) {
		const auto typed_handler = [&](auto& typed_payload) {
			return handle_untimely(typed_payload, session_id);
		};

		return std::visit(typed_handler, p.payload);
	}

	const auto next_session_id = get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
		[&](const auto& mode) {
			return mode.get_next_session_id();
		}
	);

	const bool can_still_appear = p.associated_id.value >= next_session_id.value;

	if (can_still_appear) {
		untimely_payloads.emplace_back(p);
	}

	return true;
}

template <class U>
bool client_setup::handle_untimely(U& u, const session_id_type session_id) {
	if constexpr(std::is_same_v<U, arena_player_avatar_payload>) {
		auto& new_avatar = u;

		if (new_avatar.png_bytes.size() > 0) {
			try {
				const auto size = augs::image::get_png_size(new_avatar.png_bytes);

				if (size.x > max_avatar_side_v || size.y > max_avatar_side_v) {
					LOG("The server has tried to send an avatar of size %xx%x!", size.x, size.y);
					log_malicious_server();
					return false;
				}
			}
			catch (const augs::image_loading_error& err) {
				LOG("The server has tried to send an invalid avatar!");
				LOG(err.what());
				log_malicious_server();
				return false;
			}
		}

		const auto player_id = find_by(session_id);
		ensure(logically_set(player_id));

		player_metas[player_id.value].avatar = std::move(new_avatar);
		rebuild_player_meta_viewables = true;
	}
	else {
		static_assert(always_false_v<U>);
	}

	return true;
}

void client_setup::handle_server_messages() {
	namespace N = net_messages;

	auto& message_handler = *this;

	if (vars.network_simulator.value.loss_percent >= 100.f) {
		return;
	}

	adapter->advance(client_time, message_handler);
}

#define STRESS_TEST_ARENA_SERIALIZATION 0
#if STRESS_TEST_ARENA_SERIALIZATION
#include <random>
#include "augs/readwrite/lua_file.h"
#include "augs/readwrite/to_bytes.h"
#include "augs/misc/getpid.h"
#include "game/modes/dump_for_debugging.h"
#endif

void client_setup::send_pending_commands() {
#if STRESS_TEST_ARENA_SERIALIZATION
	static auto ndt_rng = randomization(std::random_device()());

	const auto times = ndt_rng.randval(0, 4);

	if (times > 0) {
		const auto this_nickname = [&]() {
			if (const auto ch = get_viewed_character()) {
				return ch.get_name();
			}

			return std::string("noentity");
		}();

		const auto pid = augs::getpid();

		const auto preffix = this_nickname + "_srl_" + std::to_string(pid) + "_";

		const auto original_signi = scene.world.get_solvable().significant;
		const auto original_signi_bytes = augs::to_bytes(original_signi);

		const auto original_mode = current_mode;
		const auto original_mode_bytes = augs::to_bytes(original_mode);

		uint32_t exchanged_client_id = 0xdeadbeef;

		std::vector<std::byte> initial_buf;

		for (int i = 0; i < times; ++i) {
			net_messages::initial_arena_state ss;
			ss.Release();

			auto write_all = [&]() {
				return ss.write_payload(
					buffers,

					initial_arena_state_payload<true> {
						scene.world.get_solvable().significant,
						current_mode,
						exchanged_client_id,
						rcon
					}
				);
			};

			auto written = write_all();

			if (initial_buf.empty()) {
				initial_buf = *written;
			}
			else {
				if (initial_buf != *written) {
					{
						const auto& tampered_cosm = scene.world;
						const auto& tampered_mode = current_mode;

						dump_for_debugging(
							lua,
							preffix + "tampered_",
							tampered_cosm,
							tampered_mode
						);
					}

					auto as_text = [&](const auto& bytes) {
						std::string ss;

						for (const auto& b : bytes) {
							ss += std::to_string(int(b)) + "\n";
						}

						return ss;
					};

					auto make_path = [&](const auto& of) {
						return augs::path_type(preffix + of);
					};

					augs::save_as_text(make_path("tampered_bytes.txt"), as_text(*written));

					augs::save_as_text(make_path("original_bytes.txt"), as_text(initial_buf));
					augs::save_as_text(make_path("original_signi_bytes.txt"), as_text(original_signi_bytes));
					augs::save_as_text(make_path("original_mode_bytes.txt"), as_text(original_mode_bytes));

					augs::save_as_lua_table(lua, original_signi, make_path("original_signi.lua"));
					augs::save_as_lua_table(lua, original_mode, make_path("original_mode.lua"));

					ensure(false && "The serialization cycle is unstable. This might be a cause for indeterminism.");
				}
			}

			auto read_all = [&]() {
				cosmic::change_solvable_significant(
					scene.world, 
					[&](cosmos_solvable_significant& signi) {
						ss.AttachBlock(yojimbo::GetDefaultAllocator(), reinterpret_cast<uint8_t*>(initial_buf.data()), initial_buf.size());

						ss.read_payload(
							buffers,

							initial_payload {
								signi,
								current_mode,
								exchanged_client_id
							}
						);

						ss.DetachBlock();

						return changer_callback_result::DONT_REFRESH;
					}
				);
			};

			read_all();
		}
	}
#endif

	using C = client_state_type;

	const bool init_send = state == C::INITIATING_CONNECTION;

	const bool can_already_resend_settings = client_time - when_sent_client_settings > 1.0;
	const bool resend_requested_settings = can_already_resend_settings && !augs::introspective_equal(current_requested_settings, requested_settings);

	auto send_settings = [&]() {
		adapter->send_payload(
			game_channel_type::CLIENT_COMMANDS,
			std::as_const(requested_settings)
		);

		if (init_send) {
			LOG("Sent initial client configuration to the server.");
			state = client_state_type::PENDING_WELCOME;
		}
		else {
			LOG("Sent repeated client configuration to the server.");
		}

		when_sent_client_settings = client_time;
		current_requested_settings = requested_settings;
	};

	if (init_send || resend_requested_settings) {
		send_settings();
	}

	const auto& avatar_path = vars.avatar_image_path;

	if (state == C::IN_GAME) {
		if (last_sent_avatar.empty()) {
			if (!avatar_path.empty()) {
				arena_player_avatar_payload payload;

				try {
					payload.png_bytes = augs::file_to_bytes(avatar_path);
				}
				catch (...) {
					payload.png_bytes.clear();
				}

				if (payload.png_bytes.size() > 0 && payload.png_bytes.size() <= max_avatar_bytes_v) {
					const auto dummy_client_id = session_id_type::dead();

					adapter->send_payload(
						game_channel_type::COMMUNICATIONS,

						dummy_client_id,
						payload
					);
				}
				else {
					const auto reason = typesafe_sprintf(
						"The avatar file (%x) exceeds the maximum size of %x.\nSupply a less entropic PNG file.", 
						readable_bytesize(payload.png_bytes.size()), 
						readable_bytesize(max_avatar_bytes_v)
					);

					set_disconnect_reason(reason);
					disconnect();
				}
			}

			last_sent_avatar = avatar_path;
		}
	}

	if (pending_request == special_client_request::RESYNC) {
		LOG("Sending the request resync command.");

		adapter->send_payload(
			game_channel_type::CLIENT_COMMANDS,
			pending_request
		);

		pending_request = special_client_request::NONE;
	}
}

void client_setup::send_packets_if_its_time() {
	if (vars.network_simulator.value.loss_percent >= 100.f) {
		return;
	}

	adapter->send_packets();
}

void client_setup::log_malicious_server() {
	set_disconnect_reason("The client is out of date, or the server might be malicious.\nIf your game is up to date, please report this fact to the developers\n - send them the files in cache/usr/log.", true);

#if !IS_PRODUCTION_BUILD
	ensure(false && "Server has sent some invalid data.");
#endif
}

custom_imgui_result client_setup::perform_custom_imgui(
	const perform_custom_imgui_input in
) {
	using C = client_state_type;
	using namespace augs::imgui;

	arena_gui.resyncing_notifier = now_resyncing;

	const bool is_gameplay_on = adapter->is_connected() && state == C::IN_GAME;

	auto& rcon_gui = client_gui.rcon;

	if (!adapter->is_connected()) {
		rcon_gui.show = false;
	}

	if (!arena_gui.scoreboard.show && rcon_gui.show) {
		const auto window_name = "Remote Control (RCON)";

		ImGui::SetNextWindowPosCenter();

		ImGui::SetNextWindowSize((vec2(ImGui::GetIO().DisplaySize) * 0.7f).operator ImVec2(), ImGuiCond_Once);

		auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar);
		centered_text(window_name);

		if (rcon == rcon_level_type::NONE) {
			text_color("Access denied.", red);
			ImGui::Separator();
		}
		else {
			do_pretty_tabs(rcon_gui.active_pane);

			auto do_server_vars_panel = [&](
				auto& edited,
				auto& last_saved,
				auto& applying_flag
			) { 
				{
					auto child = scoped_child("settings view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
					auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

					do_server_vars(
						edited,
						last_saved
					);
				}

				{
					auto scope = scoped_child("save revert");

					ImGui::Separator();

					if (applying_flag) {
						text_color("Applying changes...", yellow);
					}
					else if (!augs::introspective_equal(last_saved, edited)) {
						if (ImGui::Button("Apply & Save")) {
							rcon_command_variant payload;
							payload = edited;

							LOG("Sending new configuration (%x)", edited_sv_solvable_vars.current_arena);

							adapter->send_payload(
								game_channel_type::COMMUNICATIONS,
								payload
							);

							applying_flag = true;
						}

						ImGui::SameLine();

						if (ImGui::Button("Revert all")) {
							edited = last_saved;
						}
					}
				}
			};

			auto do_command_button = [&](const std::string& label, const auto cmd) {
				if (ImGui::Button(label.c_str())) {
					rcon_command_variant payload;
					payload = cmd;

					adapter->send_payload(
						game_channel_type::COMMUNICATIONS,
						payload
					);

					return true;
				}

				return false;
			};

			switch (rcon_gui.active_pane) {
				case rcon_pane::ARENAS:
					do_server_vars_panel(edited_sv_solvable_vars, last_applied_sv_solvable_vars, applying_sv_solvable_vars);
					break;

				case rcon_pane::MATCH:
					using MC = match_command;

					augs::for_each_enum_except_bounds([&](const MC cmd) {
						do_command_button(format_enum(cmd), cmd);
					});

					break;

				case rcon_pane::VARS:
					do_server_vars_panel(edited_sv_vars, last_applied_sv_vars, applying_sv_vars);
					break;

				case rcon_pane::RULESETS:
					break;

				case rcon_pane::USERS:
					break;

				case rcon_pane::ADVANCED:
					using RS = rcon_commands::special;

					if (do_command_button("Shutdown server", RS::SHUTDOWN)) {
						LOG("Requesting the server to shut down");
					}

					do_command_button("Download logs", RS::DOWNLOAD_LOGS);

					break;

				default: break;
			}
		}

		ImGui::Separator();
	}

	{
		auto& chat = client_gui.chat;

		if (chat.perform_input_bar(vars.client_chat)) {
			::client_requested_chat message;

			message.target = chat.target;
			message.message = chat.current_message;

			adapter->send_payload(
				game_channel_type::COMMUNICATIONS,
				message
			);

			chat.current_message.clear();
		}
	}

	if (is_gameplay_on) {
		augs::network::enable_detailed_logs(false);

		arena_base::perform_custom_imgui(in);
	}
	else {
		ImGui::SetNextWindowPosCenter();

		ImGui::SetNextWindowSize((vec2(ImGui::GetIO().DisplaySize) * 0.3f).operator ImVec2(), ImGuiCond_FirstUseEver);

		auto print_reason_if_any = [&]() {
			if (last_disconnect_reason.empty()) {
				return;
			}

			if (print_only_disconnect_reason) {
				text(last_disconnect_reason);
			}
			else {
				text("Reason:\n\n%x", last_disconnect_reason);
			}
		};

		const auto window_name = "Connection status";
		auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

		const bool failed_after_connected = adapter->is_connecting() && state > C::INITIATING_CONNECTION;

		if (state == C::INITIATING_CONNECTION && adapter->is_connecting()) {
			text("Connecting to %x\nTime: %2f seconds", last_start.connect_address, get_current_time() - when_initiated_connection);

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Abort")) {
				disconnect();
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
		else if (failed_after_connected || adapter->has_connection_failed()) {
			if (state == C::IN_GAME) {
				text("Lost connection to the server.");
			}
			else if (state == C::INITIATING_CONNECTION) {
				text("Failed to establish connection with %x", last_start.connect_address);
			}
			else {
				text("Failed to join %x", last_start.connect_address);
			}

			print_reason_if_any();

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Retry")) {
				return custom_imgui_result::RETRY;
			}

			ImGui::SameLine();

			if (ImGui::Button("Go back")) {
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
		else if (adapter->is_disconnected()) {
			if (!print_only_disconnect_reason) {
				text("Disconnected from the server.");
			}

			print_reason_if_any();

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Go back")) {
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
		else if (adapter->is_connected()) {
			augs::network::enable_detailed_logs(false);

			text_color(typesafe_sprintf("Connected to %x.", last_start.connect_address), green);

			if (state == C::INITIATING_CONNECTION) {
				text("Initializing connection...");
			}
			else if (state == C::PENDING_WELCOME) {
				text("Sending the client configuration.");
			}
			else if (state == C::RECEIVING_INITIAL_STATE) {
				text("Receiving the initial state:");
			}
			else if (state == C::RECEIVING_INITIAL_STATE_CORRECTION) {
				text("Receiving the initial state correction:");
			}
			else {
				text("Unknown error.");
			}

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Abort")) {
				disconnect();
			}
		}
	}

	return custom_imgui_result::NONE;
}

void client_setup::apply(const config_lua_table& cfg) {
	vars = cfg.client;

	auto& r = requested_settings;
	r.chosen_nickname = vars.nickname;
	r.rcon_password = vars.rcon_password;
	r.net = vars.net;
	r.public_settings.character_input = cfg.input.character;

	adapter->set(vars.network_simulator);
}

bool client_setup::is_connected() const {
	return adapter->is_connected();
}

void client_setup::send_to_server(
	total_client_entropy& new_local_entropy
) {
	adapter->send_payload(
		game_channel_type::CLIENT_COMMANDS,
		new_local_entropy
	);
}

void client_setup::disconnect() {
	adapter->disconnect();
}

bool client_setup::is_gameplay_on() const {
	return is_connected() && state == client_state_type::IN_GAME;
}

setup_escape_result client_setup::escape() {
	if (!is_gameplay_on()) {
		return setup_escape_result::GO_TO_MAIN_MENU;
	}

	return arena_base::escape();
}

const cosmos& client_setup::get_viewed_cosmos() const {
	return get_arena_handle(get_viewed_arena_type()).get_cosmos();
}

void client_setup::update_stats(network_info& stats) const {
	stats = adapter->get_network_info();
}

augs::path_type client_setup::get_unofficial_content_dir() const {
	const auto& name = sv_solvable_vars.current_arena;

	if (name.empty()) {
		return {};
	}

	const auto paths = arena_paths(name);
	return paths.folder_path;
}

bool client_setup::is_loopback() const {
	return is_connected() && adapter->get_server_address().IsLoopback();
}

bool client_setup::handle_input_before_game(
	const handle_input_before_game_input in
) {
	if (arena_base::handle_input_before_game(in)) {
		return true;
	}

	if (client_gui.control(in)) {
		return true;
	}

	return false;
}

void client_setup::draw_custom_gui(const draw_setup_gui_input& in) const {
	using namespace augs::gui::text;

	client_gui.chat.draw_recent_messages(
		in.get_drawer(),
		vars.client_chat,
		in.config.faction_view,
		in.gui_fonts.gui,
		get_current_time()
	);

	arena_base::draw_custom_gui(in);
}

std::optional<arena_player_metas> client_setup::get_new_player_metas() {
	if (rebuild_player_meta_viewables) {
		rebuild_player_meta_viewables = false;
		return player_metas;
	}

	return std::nullopt;
}

mode_player_id client_setup::get_local_player_id() const {
	return client_player_id;
}

mode_player_id client_setup::find_by(const session_id_type id) const {
	return get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
		[&](const auto& mode) {
			return mode.lookup(id);
		}
	);
}

std::optional<session_id_type> client_setup::find_session_id(const mode_player_id id) const {
	return get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
		[&](const auto& mode) -> std::optional<session_id_type> {
			if (const auto entry = mode.find(id)) {
				return entry->session_id;
			}

			return std::nullopt;
		}
	);
}

std::optional<session_id_type> client_setup::find_local_session_id() const {
	return find_session_id(get_local_player_id());
}

template <class I, class O, class K>
auto find_in_indirectors(const I& indirectors, O& objects, const K key) -> maybe_const_ptr_t<std::is_const_v<O>, typename O::value_type> {
	if (key.indirection_index >= indirectors.size()) {
		return nullptr;
	}

	const auto& indirector = indirectors[key.indirection_index];
	using size_type = decltype(indirector.real_index);

	const bool versions_match = indirector.version == key.version && indirector.real_index != static_cast<size_type>(-1);

	if (!versions_match) {
		return nullptr;
	}

	return &objects[indirector.real_index];
}

void save_interpolations(
	interpolation_transfer_caches& caches,
	const cosmos& source
) {
	source.get_solvable().significant.entity_pools.for_each_container(
		[&](const auto& p) {
			using P = remove_cref<decltype(p)>;
			using V = typename P::mapped_type;
			using E = entity_type_of<V>;

			if constexpr(has_all_of_v<E, invariants::interpolation>) {
				auto& c = caches.get_for<E>();
				c.interpolations = p.template get_corresponding_array<components::interpolation>();
				c.indirectors = p.get_indirectors();
			}
		}
	);
}

void restore_interpolations(
	const interpolation_transfer_caches& caches,
	cosmos& target
) {
	target.for_each_having<invariants::interpolation>(
		[&](const auto& typed_adjusted) {
			using E = entity_type_of<decltype(typed_adjusted)>;

			const auto id = typed_adjusted.get_id();
			const auto& c = caches.get_for<E>();

			if (const auto entry = ::find_in_indirectors(c.indirectors, c.interpolations, id.raw)) {
				get_corresponding<components::interpolation>(typed_adjusted) = *entry;
			}
		}
	);
}

void client_setup::handle_new_session(const add_player_input& in) {
	const auto new_player = in.id;
	const auto new_session_id = find_session_id(new_player);

	ensure(new_session_id != std::nullopt);

	auto& meta = player_metas[new_player.value];
	meta.clear_session_channeled_data();
	meta.session_id = *new_session_id;

	auto untimely_handler = [&](auto& untimely) {
		if (logically_set(find_by(untimely.associated_id))) {
			const auto typed_handler = [&](auto& typed_payload) {
				return handle_untimely(typed_payload, untimely.associated_id);
			};

			const auto result = std::visit(typed_handler, untimely.payload);

			if (!result) {
				disconnect();
			}

			return true;
		}

		return false;
	};

	erase_if(untimely_payloads, untimely_handler);
	rebuild_player_meta_viewables = true;
}
