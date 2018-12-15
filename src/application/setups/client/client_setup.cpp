#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/client/client_setup.h"
#include "application/config_lua_table.h"

#include "application/network/client_adapter.hpp"
#include "application/network/net_message_translation.h"

#include "game/cosmos/change_solvable_significant.h"

#include "augs/misc/pool/pool_io.hpp"
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
	state = client_state_type::INVALID;
	client->connect(in);
	when_initiated_connection = get_current_time();
}

client_setup::~client_setup() {
	client->disconnect();
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
		sv_vars = payload;

		::choose_arena(
			lua,
			get_arena_handle(),
			sv_vars,
			initial_signi
		);

		if (state == client_state_type::PENDING_WELCOME) {
			state = client_state_type::RECEIVING_INITIAL_STATE;
		}
	}
	else if constexpr (std::is_same_v<T, initial_payload>) {
		if (state != client_state_type::RECEIVING_INITIAL_STATE) {
			log_malicious_server();
			client->disconnect();
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

		state = client_state_type::IN_GAME;
	}
	else if constexpr (std::is_same_v<T, server_step_entropy_for_client>) {
		const auto num_accepted = payload.num_entropies_accepted;
		(void)num_accepted;
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

void client_setup::send_client_commands() {
	if (client->is_connected()) {
		const bool init_send = state == client_state_type::INVALID;

		if (resend_requested_settings || init_send) {
			client->send_payload(
				game_channel_type::CLIENT_COMMANDS,
				std::as_const(requested_settings)
			);

			if (init_send) {
				state = client_state_type::PENDING_WELCOME;
			}
		}
	}
}

void client_setup::send_packets_if_its_time() {
	client->send_packets();
}

void client_setup::log_malicious_server() {

}

void client_setup::perform_custom_imgui(
	const perform_custom_imgui_input in
) {
	using C = client_state_type;
	using namespace augs::imgui;

	const bool is_in_game = client->is_connected() && state == C::IN_GAME;

	if (is_in_game) {
		arena_base::perform_custom_imgui(in);
	}
	else {
		ImGui::SetNextWindowPosCenter();

		ImGui::SetNextWindowSize((vec2(ImGui::GetIO().DisplaySize) * 0.3f).operator ImVec2(), ImGuiCond_FirstUseEver);

		const auto window_name = "Connection status";
		auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar);

		if (client->is_connecting()) {
			text("Connecting to %x\nTime: %3f seconds", last_start.ip_port, get_current_time() - when_initiated_connection);
		}
		else if (client->is_disconnected()) {
			text("Client disconnected.");
		}
		else if (client->has_connection_failed()) {
			text("The connection to %x has failed.", last_start.ip_port);
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
		}
	}
}

void client_setup::apply(const config_lua_table& cfg) {
	vars = cfg.client;

	auto& r = requested_settings;
	r.chosen_nickname = vars.nickname;
	r.net = vars.net;
	r.public_settings.mouse_sensitivity = cfg.input.mouse_sensitivity;
}
