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

using server_step_type = unsigned;

class server_setup : public default_setup_settings {
	/* This is loaded from the arena folder */
	intercosm scene;
	predefined_rulesets rulesets;

	/* Other replicated state */
	online_mode_and_rules current_mode;
	server_vars vars;

	/* The rest is server-specific */
	server_step_type current_simulation_step = 0;

	augs::serialization_buffers buffers;

	entropy_accumulator total_collected;
	entity_id viewed_character_id;

	augs::propagate_const<std::unique_ptr<server_adapter>> server;
	std::array<server_client_state, max_incoming_connections_v> clients;
	unsigned ticks_until_sending_packets = 0;
	net_time_t server_time = 0.0;

	/* No server state follows later in code. */

	augs::ref_memory_stream make_serialization_stream();

	static net_time_t get_current_time();

	template <class S, class F>
	static decltype(auto) on_mode_with_input_impl(
		S& self,
		F&& callback
	);

	void advance_internal(
		const setup_advance_input& in
	);

	void handle_client_messages(
		const setup_advance_input& in
	);

	void advance_clients_state(
		const setup_advance_input& in
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
	static constexpr bool handles_window_input = false;
	static constexpr bool has_additional_highlights = false;

	server_setup(
		sol::state& lua,
		const server_start_input&
	);

	~server_setup();

	static mode_player_id to_mode_player_id(const client_id_type&);

	auto get_audiovisual_speed() const {
		return 1.0;
	}

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	auto get_interpolation_ratio() const {
		const auto dt =  get_viewed_cosmos().get_fixed_delta().in_seconds<double>();
		return (get_current_time() - server_time) / dt;
	}

	auto get_viewed_character_id() const {
		return viewed_character_id;
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return scene.viewables;
	}

	void perform_custom_imgui() {
		return;
	}

	void customize_for_viewing(config_lua_table&) const;
	void apply(const config_lua_table&);

	void choose_arena(const std::string& name);

	bool add_to_arena(add_to_arena_input);
	void remove_from_arena(const client_id_type&);

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
			advance_internal(in);

			++current_simulation_step;

			{
				/* Process the built-in server player */
				const auto total = total_collected.extract(
					get_viewed_character(), 
					mode_player_id::machine_admin(),
					in
				);

				(void)total;
			}

			server_time += dt;
		}

		(void)callbacks;
	}

	template <class T>
	void control(const T& t) {
		total_collected.control(t);
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

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&&... args) {
		return on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&&... args) const {
		return on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}

	template <class F>
	decltype(auto) on_mode(F&& f) {
		return std::visit(std::forward<F>(f), current_mode.state);
	}

	template <class F>
	decltype(auto) on_mode(F&& f) const {
		return std::visit(std::forward<F>(f), current_mode.state);
	}

	void disconnect_and_unset(const client_id_type&);
};
