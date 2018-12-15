#pragma once
#include "augs/math/camera_cone.h"
#include "game/detail/render_layer_filter.h"
#include "application/setups/client/client_start_input.h"
#include "application/intercosm.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "application/setups/default_setup_settings.h"
#include "application/input/entropy_accumulator.h"

#include "application/setups/setup_common.h"
#include "application/setups/server/server_vars.h"

#include "application/network/network_common.h"

#include "augs/network/network_types.h"
#include "application/setups/server/server_vars.h"

#include "application/predefined_rulesets.h"
#include "application/arena/mode_and_rules.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/misc/serialization_buffers.h"

#include "view/mode_gui/arena/arena_gui_mixin.h"
#include "application/network/client_state_type.h"
#include "application/network/requested_client_settings.h"

struct config_lua_table;

class client_adapter;

enum class client_arena_type {
	PREDICTED,
	REFERENTIAL
};

class client_setup : 
	public default_setup_settings,
	public arena_gui_mixin<client_setup>
{
	using arena_base = arena_gui_mixin<client_setup>;
	friend arena_base;
	friend client_adapter;

	/* This is loaded from the arena folder */
	intercosm scene;
	cosmos_solvable_significant initial_signi;

	predefined_rulesets rulesets;

	/* Other replicated state */
	online_mode_and_rules current_mode;
	server_vars sv_vars;

	mode_player_id client_player_id;

	cosmos referential_cosmos;
	online_mode_and_rules referential_mode;

	/* The rest is client-specific */
	sol::state& lua;

	client_start_input last_start;
	client_state_type state = client_state_type::INVALID;

	client_vars vars;
	requested_client_settings requested_settings;
	bool resend_requested_settings = false;

	entropy_accumulator total_collected;
	augs::serialization_buffers buffers;

	augs::propagate_const<std::unique_ptr<client_adapter>> client;
	net_time_t client_time = 0.0;
	net_time_t when_initiated_connection = 0.0;

	/* No client state follows later in code. */

	static net_time_t get_current_time();

	template <class H, class S>
	static decltype(auto) get_arena_handle_impl(S& self, const client_arena_type t) {
		if (t == client_arena_type::PREDICTED) {
			return H {
				self.current_mode,
				self.scene,
				self.rulesets,
				self.initial_signi
			};
		}
		else {
			ensure_eq(t, client_arena_type::REFERENTIAL);

			return H {
				self.referential_mode,
				self.scene,
				self.rulesets,
				self.initial_signi
			};
		}
	}

	void handle_server_messages();
	void send_client_commands();
	void send_packets_if_its_time();

	template <class T, class F>
	message_handler_result handle_server_message(
		F&& read_payload
	);

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool has_additional_highlights = false;

	client_setup(
		sol::state& lua,
		const client_start_input&
	);

	~client_setup();

	void init_connection(const client_start_input&);

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	auto get_interpolation_ratio() const {
		const auto dt = get_viewed_cosmos().get_fixed_delta().in_seconds<double>();
		return (get_current_time() - client_time) / dt;
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

	double get_audiovisual_speed() const;
	double get_inv_tickrate() const;

	template <class C>
	void advance(
		const setup_advance_input& in,
		const C& callbacks
	) {
		const auto current_time = get_current_time();
		const auto dt = get_inv_tickrate();

		while (client_time <= current_time) {
			const auto total = total_collected.extract(
				get_viewed_character(), 
				get_local_player_id(), 
				{ in.settings, in.screen_size }
			);
			
			handle_server_messages();
			send_client_commands();
			send_packets_if_its_time();

			get_arena_handle().on_mode_with_input(
				[&](auto& typed_mode, const auto& in) {
					typed_mode.advance(in, total, callbacks);
				}
			);

			client_time += dt;
		}
	}

	template <class T>
	void control(const T& t) {
		total_collected.control(t);
	}

	void accept_game_gui_events(const game_gui_entropy_type&);

	augs::path_type get_unofficial_content_dir() const {
		return {};
	}

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void ensure_handler() {}

	mode_player_id get_local_player_id() const {
		return client_player_id;
	}

	online_arena_handle<false> get_arena_handle(client_arena_type = client_arena_type::PREDICTED);
	online_arena_handle<true> get_arena_handle(client_arena_type = client_arena_type::PREDICTED) const;

	void log_malicious_server();
};
