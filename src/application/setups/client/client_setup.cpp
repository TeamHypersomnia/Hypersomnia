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

client_setup::client_setup(
	sol::state& lua,
	const client_start_input& in
) : 
	lua(lua),
	client(std::make_unique<client_adapter>())
{
	init_connection(in);
}

void client_setup::init_connection(const client_start_input& in) {
	last_start = in;

	state = client_state_type::INVALID;
	client->connect(in);
	when_initiated_connection = get_current_time();
	receiver.clear();
	last_disconnect_reason.clear();
}

client_setup::~client_setup() {
	disconnect();
}

net_time_t client_setup::get_current_time() {
	return yojimbo_time();
}

entity_id client_setup::get_viewed_character_id() const {
	if (!is_gameplay_on()) {
		return entity_id::dead();
	}

	return get_arena_handle().on_mode(
		[&](const auto& typed_mode) {
			return typed_mode.lookup(get_local_player_id());
		}
	);
}

void client_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena client";

	if (is_gameplay_on()) {
		get_arena_handle(client_arena_type::REFERENTIAL).adjust(config.drawing);
	}
}

void client_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

client_arena_type client_setup::get_viewed_arena_type() const {
#if USE_CLIENT_PREDICTION
	return client_arena_type::PREDICTED;
#else
	return client_arena_type::REFERENTIAL;
#endif

	// TODO: For spectating the game, use the referential arena with jitter.
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

template <class T>
constexpr bool payload_easily_movable_v = !is_one_of_v<
	T,
	initial_payload
>;

template <class T, class F>
message_handler_result client_setup::handle_server_message(
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

	if constexpr (std::is_same_v<T, server_vars>) {
		const bool are_initial_vars = state == client_state_type::PENDING_WELCOME;
		const auto& new_vars = payload;

		if (are_initial_vars) {
			LOG("Received initial vars from the server");
			state = client_state_type::RECEIVING_INITIAL_STATE;
		}
		else {
			LOG("Received corrected vars from the server");
		}

		if (are_initial_vars || sv_vars.current_arena != new_vars.current_arena) {
			::choose_arena(
				lua,
				get_arena_handle(client_arena_type::REFERENTIAL),
				new_vars,
				initial_signi
			);

			/* Prepare the predicted cosmos. */
			predicted_cosmos = scene.world;
		}

		sv_vars = new_vars;
	}
	else if constexpr (std::is_same_v<T, initial_payload>) {
		if (state != client_state_type::RECEIVING_INITIAL_STATE) {
			LOG("The server has sent initial state early (state: %x). Disconnecting.", state);
			log_malicious_server();
			return abort_v;
		}

		uint32_t read_client_id;

		cosmic::change_solvable_significant(
			scene.world, 
			[&](cosmos_solvable_significant& signi) {
				read_payload(
					buffers,

					initial_payload {
						signi,
						current_mode,
						read_client_id
					}
				);

				return changer_callback_result::REFRESH;
			}
		);

		client_player_id = static_cast<mode_player_id>(read_client_id);

		LOG("Received initial state from the server at step: %x.", scene.world.get_timestamp().step);
		LOG("Received client id: %x", client_player_id);

		state = client_state_type::IN_GAME;
	}
	else if constexpr (std::is_same_v<T, prestep_client_context>) {
		if (state != client_state_type::IN_GAME) {
			LOG("The server has sent prestep context too early (state: %x). Disconnecting.", state);

			log_malicious_server();
			return abort_v;
		}

		receiver.acquire_next_server_entropy(payload);
	}
	else if constexpr (std::is_same_v<T, networked_server_step_entropy>) {
		if (state != client_state_type::IN_GAME) {
			LOG("The server has sent entropy too early (state: %x). Disconnecting.", state);

			log_malicious_server();
			return abort_v;
		}

		{
			auto mode_id_to_entity_id = [&](const mode_player_id& mode_id) {
				return get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
					[&](const auto& typed_mode) {
						return typed_mode.lookup(mode_id);
					}
				);
			};

			receiver.acquire_next_server_entropy(payload.unpack(mode_id_to_entity_id));
		}

		const auto& max_commands = vars.max_buffered_server_commands;
		const auto num_commands = receiver.incoming_entropies.size();

		if (num_commands > max_commands) {
			last_disconnect_reason = typesafe_sprintf(
				"Number of buffered server commands (%x) exceeded max_buffered_server_commands (%x).", 
				num_commands,
				max_commands
			);

			return abort_v;
		}

		//LOG("Received %x th entropy from the server", receiver.incoming_entropies.size());
		//LOG_NVPS(payload.num_entropies_accepted);
	}
	else {
		static_assert(always_false_v<T>, "Unhandled payload type.");
	}

	return message_handler_result::CONTINUE;
}

void client_setup::handle_server_messages() {
	namespace N = net_messages;

	auto& message_handler = *this;
	client->advance(client_time, message_handler);
}

void client_setup::send_pending_commands() {
	using C = client_state_type;

	const bool init_send = state == C::INVALID;

	if (resend_requested_settings || init_send) {
		client->send_payload(
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
	}
}

void client_setup::send_packets_if_its_time() {
	client->send_packets();
}

void client_setup::log_malicious_server() {
	last_disconnect_reason = "The client is broken or the server might be malicious.\nPlease report this fact to the developers - send them the files in cache/usr/log.";

#if !IS_PRODUCTION_BUILD
	ensure(false && "Server has sent some invalid data.");
#endif
}

custom_imgui_result client_setup::perform_custom_imgui(
	const perform_custom_imgui_input in
) {
	using C = client_state_type;
	using namespace augs::imgui;

	const bool is_gameplay_on = client->is_connected() && state == C::IN_GAME;

	if (is_gameplay_on) {
		arena_base::perform_custom_imgui(in);
	}
	else {
		ImGui::SetNextWindowPosCenter();

		ImGui::SetNextWindowSize((vec2(ImGui::GetIO().DisplaySize) * 0.3f).operator ImVec2(), ImGuiCond_FirstUseEver);

		auto print_reason_if_any = [&]() {
			if (last_disconnect_reason.empty()) {
				return;
			}

			text("Reason: \n%x", last_disconnect_reason);
		};

		const auto window_name = "Connection status";
		auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

		if (client->is_connecting()) {
			text("Connecting to %x\nTime: %2f seconds", last_start.ip_port, get_current_time() - when_initiated_connection);

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Abort")) {
				disconnect();
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
		else if (client->has_connection_failed()) {
			if (state == C::IN_GAME) {
				text("You have been kicked from the server.");

				print_reason_if_any();

				text("\n");
				ImGui::Separator();

				if (ImGui::Button("Go back")) {
					return custom_imgui_result::GO_TO_MAIN_MENU;
				}
			}
			else {
				text("Failed to establish connection with %x.", last_start.ip_port);

				print_reason_if_any();

				text("\n");
				ImGui::Separator();

				if (ImGui::Button("Retry")) {
					init_connection(last_start);
				}

				ImGui::SameLine();

				if (ImGui::Button("Go back")) {
					return custom_imgui_result::GO_TO_MAIN_MENU;
				}
			}
		}
		else if (client->is_disconnected()) {
			text("Connection has ended.");

			print_reason_if_any();

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Go back")) {
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
		else if (client->is_connected()) {
			text_color(typesafe_sprintf("Connected to %x.", last_start.ip_port), green);

			if (state == C::PENDING_WELCOME) {
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
	r.net = vars.net;
	r.public_settings.mouse_sensitivity = cfg.input.mouse_sensitivity;

	client->set(vars.network_simulator);
}

bool client_setup::is_connected() const {
	return client->is_connected();
}

void client_setup::send_to_server(
	total_client_entropy& new_local_entropy
) {
	client->send_payload(
		game_channel_type::CLIENT_COMMANDS,
		new_local_entropy
	);
}

void client_setup::disconnect() {
	client->disconnect();
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
	stats = client->get_network_info();
}

augs::path_type client_setup::get_unofficial_content_dir() const {
	const auto& name = sv_vars.current_arena;

	if (name.empty()) {
		return {};
	}

	const auto paths = arena_paths(name);
	return paths.folder_path;
}

void client_setup::draw_custom_gui(const draw_setup_gui_input& in) const {
	const auto& self = *this;

	if (!self.is_gameplay_on()) {
		return;
	}

	const auto game_screen_top = 0.f;

	const auto draw_mode_in = draw_mode_gui_input { 
		game_screen_top,
		self.get_local_player_id(), 
		in.images_in_atlas,
		in.config
	};

	// TODO: When spectating, draw everything through referential arena.

	self.get_arena_handle(client_arena_type::REFERENTIAL).on_mode_with_input(
		[&](const auto& typed_mode, const auto& mode_input) {
			arena_gui.draw_mode_gui(in, draw_mode_in, typed_mode, mode_input, prediction_input::referential(self.get_viewed_character()));
		}
	);

	self.get_arena_handle(client_arena_type::PREDICTED).on_mode_with_input(
		[&](const auto& typed_mode, const auto& mode_input) {
			arena_gui.draw_mode_gui(in, draw_mode_in, typed_mode, mode_input, prediction_input::predicted(self.get_viewed_character()));
		}
	);
}
