#pragma once
#include "application/setups/server/server_start_input.h"
#include "augs/math/camera_cone.h"
#include "game/detail/render_layer_filter.h"
#include "application/setups/server/server_start_input.h"
#include "application/intercosm.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "application/setups/default_setup_settings.h"
#include "application/input/entropy_accumulator.h"

#include "application/setups/setup_common.h"
#include "game/modes/all_mode_includes.h"
#include "game/modes/mode_entropy.h"
#include "game/modes/ruleset_id.h"

#include "augs/network/network_types.h"
#include "application/setups/server/server_vars.h"
#include "application/setups/server/server_client_state.h"
#include "application/predefined_rulesets.h"
#include "application/arena/mode_and_rules.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/misc/serialization_buffers.h"

#include "application/network/server_step_entropy.h"
#include "application/arena/arena_handle.h"

#include "application/setups/client/client_gui.h"

struct config_lua_table;
struct draw_setup_gui_input;

namespace net_messages {
	struct client_welcome;
}

struct add_to_arena_input {
	client_id_type client_id;
	entity_name_str nickname;
};

class server_adapter;

using server_step_type = uint32_t;

template <bool C>
using server_arena_handle = basic_arena_handle<C, online_mode_and_rules>;

class server_setup : public default_setup_settings {
	/* This is loaded from the arena folder */
	intercosm scene;
	cosmos_solvable_significant initial_signi;

	predefined_rulesets rulesets;

	/* Other replicated state */
	online_mode_and_rules current_mode;
	server_vars vars;

	/* The rest is server-specific */
	sol::state& lua;

	server_step_type current_simulation_step = 0;

	augs::serialization_buffers buffers;

	entropy_accumulator local_collected;
	server_step_entropy step_collected;

	augs::propagate_const<std::unique_ptr<server_adapter>> server;
	std::array<server_client_state, max_incoming_connections_v> clients;
	unsigned ticks_until_sending_packets = 0;
	net_time_t server_time = 0.0;

	client_gui_state admin_client_gui;
	/* No server state follows later in code. */

	augs::ref_memory_stream make_serialization_stream();

	static net_time_t get_current_time();

	template <class H, class S>
	static decltype(auto) get_arena_handle_impl(S& self) {
		return H {
			self.current_mode,
			self.scene,
			self.rulesets,
			self.initial_signi
		};
	}

	void handle_client_messages();
	void advance_clients_state();
	void send_server_step_entropies();
	void send_packets_if_its_time();

	void accept_entropy_of_client(
		const mode_player_id,
		const total_client_entropy&
	);

	friend server_adapter;

	template <class T, class F>
	message_handler_result handle_client_message(
		const client_id_type&, 
		F&& read_payload
	);

	void init_client(const client_id_type&);
	void unset_client(const client_id_type&);

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool has_additional_highlights = false;

	server_setup(
		sol::state& lua,
		const server_start_input&,
		const server_vars& 
	);

	~server_setup();

	static mode_player_id to_mode_player_id(const client_id_type&);

	double get_audiovisual_speed() const;

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	auto get_interpolation_ratio() const {
		const auto dt =  get_viewed_cosmos().get_fixed_delta().in_seconds<double>();
		return (get_current_time() - server_time) / dt;
	}

	entity_id get_viewed_character_id() const;

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return scene.viewables;
	}

	void perform_custom_imgui(perform_custom_imgui_input);

	void customize_for_viewing(config_lua_table&) const;

	void apply(const config_lua_table&);
	void apply(const server_vars&, bool force);

	void choose_arena(const std::string& name);

	std::string describe_client(const client_id_type id) const;
	void log_malicious_client(const client_id_type id);

	auto escape() {
		return setup_escape_result::IGNORE;
	}

	double get_inv_tickrate() const;

	template <class C>
	void advance(
		const setup_advance_input& in,
		const C& callbacks
	) {
		const auto current_time = get_current_time();
		const auto dt = get_inv_tickrate();

		while (server_time <= current_time) {
			step_collected.clear();

			handle_client_messages();
			advance_clients_state();
			send_server_step_entropies();
			send_packets_if_its_time();

			/* Extract entropy from the built-in server player */
			{
				const auto admin_entropy = local_collected.extract(
					get_viewed_character(), 
					mode_player_id::machine_admin(),
					{ in.settings, in.screen_size }
				);

				step_collected += admin_entropy;
			}

			get_arena_handle().on_mode_with_input(
				[&](auto& typed_mode, const auto& in) {
					typed_mode.advance(in, step_collected, callbacks);
				}
			);

			++current_simulation_step;
			server_time += dt;

			step_collected.clear();
		}
	}

	template <class T>
	void control(const T& t) {
		local_collected.control(t);
	}

	void accept_game_gui_events(const game_gui_entropy_type&);

	std::optional<camera_eye> find_current_camera_eye() const {
		return std::nullopt;
	}

	augs::path_type get_unofficial_content_dir() const {
		return {};
	}

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void draw_custom_gui(const draw_setup_gui_input&) {}

	void ensure_handler() {}

	server_arena_handle<false> get_arena_handle();
	server_arena_handle<true> get_arena_handle() const;

	void disconnect_and_unset(const client_id_type&);

	bool handle_input_before_imgui(
		handle_input_before_imgui_input
	);

	bool handle_input_before_game(
		handle_input_before_game_input
	);
};
