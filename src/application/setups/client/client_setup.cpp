#include "augs/misc/pool/pool_io.hpp"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/client/client_setup.h"
#include "application/config_lua_table.h"

#include "application/network/client_adapter.hpp"
#include "application/network/net_message_translation.h"
#include "application/setups/client/demo_step.h"
#include "application/network/net_message_readwrite.h"
#include "augs/templates/thread_templates.h"

#include "game/cosmos/change_solvable_significant.h"

#include "augs/readwrite/memory_stream.h"

#include "application/arena/choose_arena.h"

#include "augs/filesystem/file.h"

#include "augs/templates/introspection_utils/introspective_equal.h"
#include "augs/gui/text/printer.h"
#include "application/network/payload_easily_movable.h"
#include "augs/misc/readable_bytesize.h"
#include "game/cosmos/for_each_entity.h"
#include "game/cosmos/entity_type_traits.h"

#include "application/gui/config_nvp.h"

#include "application/setups/client/rcon_pane.h"
#include "application/gui/pretty_tabs.h"

#include "application/gui/client/rcon_gui.hpp"
#include "application/arena/arena_handle.hpp"
#include "application/setups/client/demo_paths.h"
#include "augs/misc/time_utils.h"
#include "application/network/net_message_serializers.h"
#include "augs/readwrite/byte_file.h"
#include "application/gui/client/demo_player_gui.hpp"

#include "application/setups/client/handle_server_payload.hpp"
#include "application/network/resolve_address.h"
#include "augs/network/netcode_sockets.h"
#include "application/network/resolve_address_result.h"
#include "application/nat/nat_puncher_client.h"
#include "augs/network/netcode_utils.h"

void client_demo_player::play_demo_from(const augs::path_type& p) {
	source_path = p;
	auto source = augs::open_binary_input_stream(source_path);

	augs::read_bytes(source, meta);
	augs::read_vector_until_eof(source, demo_steps);

	gui.open();
}

bool client_demo_player::control(const handle_input_before_game_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto& state = in.common_input_state;
	const bool has_alt{ state[key::LALT] };

	const auto ch = in.e.get_key_change();

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (in.e.was_any_key_pressed()) {
			switch (key) {
				case key::NUMPAD0: set_speed(1.0); return true;
				case key::NUMPAD1: set_speed(0.01); return true;
				case key::NUMPAD2: set_speed(0.05); return true;
				case key::NUMPAD3: set_speed(0.1); return true;
				case key::NUMPAD4: set_speed(0.5); return true;
				case key::NUMPAD5: set_speed(2.0); return true;
				case key::NUMPAD6: set_speed(4.0); return true;
				case key::NUMPAD7: set_speed(10.0); return true;
				case key::SPACE: paused = !paused; return true;
				case key::L: paused = false; return true;
				case key::ADD: seek_forward(1); return true;
				case key::SUBTRACT: seek_backward(1); return true;
				default: break;
			}

			if (has_alt) {
				switch (key) {
					case key::P: gui.show = !gui.show; return true;
					default: break;
				}
			}
		}

	}

	return false;
}

void client_setup::handle_server_messages_from(const demo_step& step) {
	for (auto& s : step.serialized_messages) {
		auto read_callback = [this](auto& typed_msg) -> message_handler_result {
			using net_message_type = remove_cref<decltype(typed_msg)>;

			auto read_payload_into = [&](auto&&... args) {
				return typed_msg.read_payload(
					std::forward<decltype(args)>(args)...
				);
			};

			using P = payload_of_t<net_message_type>;

			return handle_server_payload<remove_cref<P>>(std::move(read_payload_into));
		};

		try {
			const auto result = ::on_read_net_message(s, read_callback);

			if (result == message_handler_result::ABORT_AND_DISCONNECT) {
				disconnect();
				return;
			}
		}
		catch (const augs::stream_read_error& err) {
			set_demo_failed_reason(err.what());
			disconnect();
			break;
		}
	}
}

template <class T>
void client_setup::handle_server_message(T& message) {
	if (is_recording()) {
		auto bytes = ::net_message_to_bytes(message);
		get_currently_recorded_step().serialized_messages.emplace_back(std::move(bytes));
	}
}

void client_setup::play_demo_from(const augs::path_type& p) {
	demo_player.play_demo_from(p);
}

void client_setup::flush_demo_steps() {
	if (unflushed_demo_steps.empty()) {
		return;
	}

	when_last_flushed_demo = client_time;

	std::swap(demo_steps_being_flushed, unflushed_demo_steps);

	future_flushed_demo = launch_async(
		[&]() {
			auto out = augs::with_exceptions<std::ofstream>();
			out.open(recorded_demo_path, std::ios::out | std::ios::binary | std::ios::app);

			if (!was_demo_meta_written) {
				demo_file_meta meta;
				meta.server_address = last_addr.address;
				meta.version = hypersomnia_version();
				augs::write_bytes(out, meta);

				const auto version_info_path = augs::path_type(recorded_demo_path).replace_extension(".version.txt");
				augs::save_as_text(version_info_path, meta.version.get_summary());

				was_demo_meta_written = true;
			}

			for (const auto& s : demo_steps_being_flushed) {
				augs::write_bytes(out, s);
			}

			out.flush();
			demo_steps_being_flushed.clear();
		}
	);
}

bool client_setup::is_replaying() const {
	return !demo_player.source_path.empty();
}

bool client_setup::is_paused() const {
	return demo_player.is_paused();
}

bool client_setup::demo_flushing_finished() const {
	return !future_flushed_demo.valid();
}

bool client_setup::is_recording() const {
	return !recorded_demo_path.empty();
}

demo_step& client_setup::get_currently_recorded_step() {
	return unflushed_demo_steps.back();
}

void client_setup::record_demo_to(const augs::path_type& p) {
	recorded_demo_path = p;
	when_last_flushed_demo = client_time;
}

template <class... Args>
bool client_setup::send_payload(Args&&... args) {
	if (is_replaying()) {
		return true;
	}

	return adapter->send_payload(std::forward<Args>(args)...);
}

client_setup::client_setup(
	sol::state& lua,
	const client_start_input& in,
	const client_vars& initial_vars,
	const nat_detection_settings& nat_detection,
	port_type preferred_binding_port
) : 
	lua(lua),
	last_addr(in.get_address_and_port()),
	vars(initial_vars),
	adapter(std::make_unique<client_adapter>(preferred_binding_port)),
	client_time(get_current_time()),
	when_initiated_connection(get_current_time()),
	nat(std::make_unique<nat_puncher_client>())
{
	LOG("Initializing connection with %x", last_addr.address);

	const auto& input_demo_path = in.replay_demo;

	if (!input_demo_path.empty() && in.chosen_address_type == connect_address_type::REPLAY) {
		const auto error = typesafe_sprintf(
			"Failed to open demo file:\n%x", 
			input_demo_path
		);

		try {
			play_demo_from(input_demo_path);
		}
		catch (const augs::file_open_error& err) {
			set_disconnect_reason(error + "\n" + err.what(), true);
			disconnect();
		}
		catch (const augs::stream_read_error& err) {
			set_disconnect_reason(error + "\n" + err.what(), true);
			disconnect();
		}
	}
	else {
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

			const auto resolution = adapter->connect(last_addr);

			if (resolution.result == resolve_result_type::COULDNT_RESOLVE_HOST) {
				const auto reason = typesafe_sprintf(
					"Couldn't resolve host: %x", 
					resolution.host
				);

				set_disconnect_reason(reason);
			}
			else if (resolution.result == resolve_result_type::INVALID_ADDRESS) {
				const auto reason = typesafe_sprintf(
					"The address: \"%x\" is invalid!", last_addr.address
				);

				set_disconnect_reason(reason);
			}
			else {
				resolved_server_address = resolution.addr;
				nat->resolve_relay_host(nat_detection.port_probing_host);

				ensure_eq(resolution.result, resolve_result_type::OK);

				const auto new_demo_fname = augs::date_time().get_readable_for_file() + ".dem";
				const auto new_demo_path = augs::path_type(DEMOS_DIR) / new_demo_fname;

				record_demo_to(new_demo_path);
			}
		}
	}
}

client_setup::~client_setup() {
	LOG("Client setup dtor");
	disconnect();

	augs::network::enable_detailed_logs(false);

	wait_for_demo_flush();
	flush_demo_steps();
	wait_for_demo_flush();
}

net_time_t client_setup::get_current_time() {
	return yojimbo_time();
}

entity_id client_setup::get_controlled_character_id() const {
	if (!is_gameplay_on()) {
		return entity_id::dead();
	}

	if (is_replaying()) {
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

	if (is_replaying()) {
		config.arena_mode_gui.show_spectator_overlay = demo_player.gui.show_spectator_overlay;
		config.client.spectated_arena_type = demo_player.gui.shown_arena_type;

		if (is_paused()) {
			config.interpolation.enabled = false;
		}
	}
}

void client_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

bool client_setup::is_spectating_referential() const {
	const bool should_spectator_be_drawn = get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
		[this](const auto& typed_mode) {
			return arena_gui.spectator.should_be_drawn(typed_mode);
		}
	);
	
	return vars.spectated_arena_type == client_arena_type::REFERENTIAL && should_spectator_be_drawn;
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

	return get_arena_handle(client_arena_type::REFERENTIAL).get_inv_tickrate();
}

double client_setup::get_audiovisual_speed() const {
	if (!is_gameplay_on()) {
		return 1.0;
	}

	auto mult = 1.0;

	if (is_replaying()) {
		mult = demo_player.get_speed();
	}

	return mult * get_arena_handle().get_audiovisual_speed();
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

		if (new_avatar.image_bytes.size() > 0) {
			try {
				const auto size = augs::image::get_size(new_avatar.image_bytes);

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

void client_setup::handle_server_payloads() {
	namespace N = net_messages;

	if (vars.network_simulator.value.loss_percent >= 100.f) {
		return;
	}

	auto& message_handler = *this;

	adapter->advance(client_time, message_handler);
}

void client_setup::advance_demo_recorder() {
	++recorded_demo_step;

	if (client_time - when_last_flushed_demo > vars.flush_demo_to_disk_once_every_secs) {
		if (::valid_and_is_ready(future_flushed_demo)) {
			future_flushed_demo.get();
		}

		if (demo_flushing_finished()) {
			flush_demo_steps();
		}
	}
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
						get_rcon_level()
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
		send_payload(
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
					payload.image_bytes = augs::file_to_bytes(avatar_path);
				}
				catch (...) {
					payload.image_bytes.clear();
				}

				if (payload.image_bytes.size() > 0 && payload.image_bytes.size() <= max_avatar_bytes_v) {
					const auto dummy_client_id = session_id_type::dead();

					send_payload(
						game_channel_type::COMMUNICATIONS,

						dummy_client_id,
						payload
					);
				}
				else {
					const auto reason = typesafe_sprintf(
						"The avatar file (%x) exceeds the maximum size of %x.\nSupply a less entropic image file.", 
						readable_bytesize(payload.image_bytes.size()), 
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

		send_payload(
			game_channel_type::CLIENT_COMMANDS,
			pending_request
		);

		pending_request = special_client_request::NONE;
	}
}

const netcode_socket_t* client_setup::find_underlying_socket() const {
	return adapter->find_underlying_socket();
}

void client_setup::punch_this_server_if_required() {
	if (is_replaying()) {
		return;
	}

	if (!adapter->is_connecting()) {
		return;
	}

	nat->advance_relay_host_resolution();

	if (!nat->relay_host_resolved()) {
		return;
	}

	if (auto socket = adapter->find_underlying_socket()) {
		const auto nat_request_interval = 0.2;

		if (try_fire_interval(nat_request_interval, when_sent_nat_punch_request, client_time)) {
			LOG("Punching %x simultaneously with the client connection attempt, just in case.", ::ToString(resolved_server_address));
			nat->punch_this_server(*socket, resolved_server_address);
		}
	}
}

void client_setup::send_packets_if_its_time() {
	if (is_replaying()) {
		return;
	}

	if (vars.network_simulator.value.loss_percent >= 100.f) {
		return;
	}

	adapter->send_packets();
}

void client_setup::log_malicious_server() {
	set_disconnect_reason("The client is out of date, or the server might be malicious.\nIf your game is up to date, please report this fact to the developers\n - send them the files in the \"logs\" folder.", true);

#if !IS_PRODUCTION_BUILD
	ensure(false && "Server has sent some invalid data.");
#endif
}

void client_setup::perform_chat_input_bar() {
	auto& chat = client_gui.chat;

	if (chat.perform_input_bar(vars.client_chat)) {
		::client_requested_chat message;

		message.target = chat.target;
		message.message = chat.current_message;

		send_payload(
			game_channel_type::COMMUNICATIONS,
			message
		);

		chat.current_message.clear();
	}
}

void client_setup::snap_interpolation_of_viewed() {
	auto snap_for = [&](const auto arena_type) {
		snap_interpolated_to_logical(get_arena_handle(arena_type).get_cosmos());
	};

	snap_for(client_arena_type::PREDICTED);
	snap_for(client_arena_type::REFERENTIAL);
}

void client_setup::perform_demo_player_imgui(augs::window& window) {
	demo_player.gui.perform(window, demo_player);
	
	auto& pending_snap = demo_player.gui.pending_interpolation_snap;

	if (pending_snap) {
		snap_interpolation_of_viewed();
		pending_snap = false;
	}
}

custom_imgui_result client_setup::perform_custom_imgui(
	const perform_custom_imgui_input in
) {
	using C = client_state_type;
	using namespace augs::imgui;

	arena_gui.resyncing_notifier = now_resyncing;

	const bool is_gameplay_on = is_connected() && state == C::IN_GAME;

	auto& rcon_gui = client_gui.rcon;

	if (!is_connected()) {
		rcon_gui.show = false;
	}

	if (!arena_gui.scoreboard.show && rcon_gui.show) {
		auto on_new_payload = [&](const auto& new_payload) {
			rcon_command_variant payload;
			payload = new_payload;

			send_payload(
				game_channel_type::COMMUNICATIONS,
				payload
			);
		};

		const bool has_maintenance = true;

		perform_rcon_gui(
			rcon_gui,
			has_maintenance,
			on_new_payload
		);
	}

	perform_chat_input_bar();

	if (is_replaying()) {
		perform_demo_player_imgui(in.window);
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

#if 0
		if (is_replaying() && demo_replay_failed_reason.size() > 0) {
			text("Error during demo replay:");
			text(demo_replay_failed_reason);

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Go back")) {
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
		else
#endif
		if (is_connected()) {
			augs::network::enable_detailed_logs(false);

			text_color(typesafe_sprintf("Connected to %x.", last_addr.address), green);

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
		else if (state == C::INITIATING_CONNECTION && adapter->is_connecting()) {
			text("Connecting to %x\nTime: %2f seconds", last_addr.address, get_current_time() - when_initiated_connection);

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Abort")) {
				disconnect();
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
		else if (
			const bool failed_after_connected = adapter->is_connecting() && state > C::INITIATING_CONNECTION; 
			failed_after_connected || adapter->has_connection_failed()
		) {
			if (state == C::IN_GAME) {
				text("Lost connection to the server.");
			}
			else if (state == C::INITIATING_CONNECTION) {
				text("Failed to establish connection with %x", last_addr.address);
			}
			else {
				text("Failed to join %x", last_addr.address);
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
	}

	return custom_imgui_result::NONE;
}

void client_setup::apply(const config_lua_table& cfg) {
	vars = cfg.client;

	if (is_replaying()) {
		return;
	}

	auto& r = requested_settings;
	r.chosen_nickname = vars.nickname;
	r.rcon_password = vars.rcon_password;
	r.net = vars.net;
	r.public_settings.character_input = cfg.input.character;

	adapter->set(vars.network_simulator);
}

bool client_setup::is_connected() const {
	if (is_replaying()) {
		return true;
	}

	return adapter->is_connected();
}

void client_setup::send_to_server(
	total_client_entropy& new_local_entropy
) {
	send_payload(
		game_channel_type::CLIENT_COMMANDS,
		new_local_entropy
	);
}

void client_setup::disconnect() {
	if (is_replaying()) {
		demo_player.source_path.clear();
		return;
	}

	adapter->disconnect();
}

bool client_setup::is_gameplay_on() const {
	return is_connected() && state == client_state_type::IN_GAME;
}

setup_escape_result client_setup::escape() {
	if (!is_gameplay_on()) {
		return setup_escape_result::GO_TO_MAIN_MENU;
	}

	if (arena_gui.escape()) {
		return setup_escape_result::JUST_FETCH;
	}

	if (is_replaying() && !is_paused()) {
		demo_player.pause();
		return setup_escape_result::JUST_FETCH;
	}

	return setup_escape_result::LAUNCH_INGAME_MENU;
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

bool client_setup::handle_input_before_game(
	const handle_input_before_game_input in
) {
	if (arena_base::handle_input_before_game(in)) {
		return true;
	}

	if (client_gui.control(in)) {
		return true;
	}

	if (is_replaying()) {
		if (demo_player.control(in)) {
			return true;
		}

		const auto& state = in.common_input_state;
		const auto& e = in.e;

		if (e.was_any_key_pressed()) {
			using namespace augs::event::keys;

			const auto k = e.data.key.key;

			auto forward = [&](const auto& secs) {
				demo_player.seek_forward(secs / get_inv_tickrate());
			};

			auto backward = [&](const auto& secs) {
				demo_player.seek_backward(secs / get_inv_tickrate());
			};


			const bool has_shift{ state[key::LSHIFT] || state[key::RSHIFT] };

			switch (k) {
				case key::RIGHT: forward(has_shift ? 1 : 5); return true;
				case key::LEFT: backward(has_shift ? 1 : 5); return true;
				case key::UP: forward(has_shift ? 5 : 10); return true;
				case key::DOWN: backward(has_shift ? 5 : 10); return true;
				default: break;
			}
		}

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
				return entry->session.id;
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

bool client_setup::requires_cursor() const {
	return arena_base::requires_cursor() || client_gui.requires_cursor() || demo_player.gui.requires_cursor();
}

void client_setup::ensure_handler() {
	wait_for_demo_flush();
	flush_demo_steps();
}

void client_setup::wait_for_demo_flush() {
	if (future_flushed_demo.valid()) {
		future_flushed_demo.wait();
	}
}
