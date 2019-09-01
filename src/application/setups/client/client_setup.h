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
#include "application/setups/server/rcon_level.h"

#include "application/predefined_rulesets.h"
#include "application/arena/mode_and_rules.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/misc/serialization_buffers.h"

#include "view/mode_gui/arena/arena_gui_mixin.h"
#include "view/audiovisual_state/audiovisual_post_solve_settings.h"

#include "application/network/client_state_type.h"
#include "application/network/requested_client_settings.h"

#include "application/network/simulation_receiver.h"
#include "application/session_profiler.h"
#include "application/setups/client/lag_compensation_settings.h"

#include "augs/misc/getpid.h"
#include "game/modes/dump_for_debugging.h"

#include "view/client_arena_type.h"
#include "application/network/special_client_request.h"
#include "application/gui/client/rcon_gui.h"
#include "application/gui/client/chat_gui.h"
#include "application/gui/client/client_gui_state.h"
#include "view/mode_gui/arena/arena_player_meta.h"
#include "augs/texture_atlas/loaded_png_vector.h"

struct config_lua_table;

class client_adapter;

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
	server_solvable_vars sv_solvable_vars;

	server_vars last_applied_sv_vars;
	server_vars edited_sv_vars;

	server_solvable_vars last_applied_sv_solvable_vars;
	server_solvable_vars edited_sv_solvable_vars;

	bool applying_sv_vars = false;
	bool applying_sv_solvable_vars = false;

	mode_player_id client_player_id;

	cosmos predicted_cosmos;
	online_mode_and_rules predicted_mode;

	special_client_request pending_request = special_client_request::NONE;
	bool now_resyncing = false;

	arena_player_metas player_metas;

	/* The rest is client-specific */
	sol::state& lua;

	simulation_receiver receiver;

	address_and_port last_start;
	client_state_type state = client_state_type::INITIATING_CONNECTION;

	client_vars vars;
	requested_client_settings requested_settings;
	requested_client_settings current_requested_settings;
	rcon_level_type rcon = rcon_level_type::NONE;

	entropy_accumulator total_collected;
	augs::serialization_buffers buffers;

	augs::propagate_const<std::unique_ptr<client_adapter>> adapter;
	net_time_t client_time = 0.0;
	net_time_t when_initiated_connection = 0.0;
	net_time_t when_sent_client_settings = 0.0;

	double default_inv_tickrate = 1 / 128.0;

	std::string last_disconnect_reason;
	bool print_only_disconnect_reason = false;

	bool rebuild_player_meta_viewables = false;
	augs::path_type last_sent_avatar;
	client_gui_state client_gui;

	using untimely_payload_variant = std::variant<arena_player_avatar_payload>;

	struct untimely_payload {
		session_id_type associated_id;
		untimely_payload_variant payload;
	};

	std::vector<untimely_payload> untimely_payloads;
	/* No client state follows later in code. */

	template <class U>
	bool handle_untimely(U&, session_id_type);

	bool push_or_handle(untimely_payload&);
	void handle_new_session(const add_player_input& in);

	template <class T>
	void set_disconnect_reason(T&& reason, bool print_only_reason = false) {
		last_disconnect_reason = std::forward<T>(reason);
		print_only_disconnect_reason = print_only_reason;
	}

	static net_time_t get_current_time();

	template <class H, class S>
	static decltype(auto) get_arena_handle_impl(S& self, const client_arena_type t) {
		if (t == client_arena_type::PREDICTED) {
			return H {
				self.predicted_mode,
				self.scene,
				self.predicted_cosmos,
				self.rulesets,
				self.initial_signi
			};
		}
		else {
			ensure_eq(t, client_arena_type::REFERENTIAL);

			return H {
				self.current_mode,
				self.scene,
				self.scene.world,
				self.rulesets,
				self.initial_signi
			};
		}
	}

	void handle_server_messages();
	void send_pending_commands();
	void send_packets_if_its_time();

	template <class T, class F>
	message_handler_result handle_server_message(
		F&& read_payload
	);

	void send_to_server(total_client_entropy&);

	client_arena_type get_viewed_arena_type() const;

	auto get_controlled_character() const {
		return get_viewed_cosmos()[get_controlled_character_id()];
	}
public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool has_additional_highlights = false;

	client_setup(
		sol::state& lua,
		const client_start_input&,
		const client_vars& initial_vars
	);

	~client_setup();

	const cosmos& get_viewed_cosmos() const;

	auto get_interpolation_ratio() const {
		const auto dt = get_viewed_cosmos().get_fixed_delta().in_seconds<double>();
		return std::min(1.0, (get_current_time() - client_time) / dt);
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	entity_id get_controlled_character_id() const;

	const auto& get_viewable_defs() const {
		return scene.viewables;
	}

	custom_imgui_result perform_custom_imgui(perform_custom_imgui_input);

	void customize_for_viewing(config_lua_table&) const;

	void apply(const config_lua_table&);

	double get_audiovisual_speed() const;
	double get_inv_tickrate() const;

	auto make_accumulator_input(const client_advance_input& in) {
		auto accumulator_in = in.make_accumulator_input();
		accumulator_in.settings.character = current_requested_settings.public_settings.character_input;
		return accumulator_in;
	}

	template <class Callbacks>
	void advance(
		const client_advance_input& in,
		const Callbacks& callbacks
	) {
		const auto referential_solve_settings = [&]() {
			solve_settings out;
			out.effect_prediction = in.lag_compensation.effect_prediction;
			return out;
		}();

		const auto repredicted_solve_settings = [&]() {
			solve_settings out;
			out.effect_prediction = in.lag_compensation.effect_prediction;

			if (in.lag_compensation.confirm_controlled_character_death) {
				out.disable_knockouts = get_viewed_character();
			}

			out.simulate_decorative_organisms = in.lag_compensation.simulate_decorative_organisms_during_reconciliation;

			return out;
		}();

		const auto predicted_solve_settings = [&]() {
			auto out = repredicted_solve_settings;
			out.simulate_decorative_organisms = true;
			return out;
		}();

		auto schedule_reprediction_if_inconsistent = [&](const auto result) {
			if (result.state_inconsistent) {
				receiver.schedule_reprediction = true;
			}
		};

		auto& performance = in.network_performance;

		const auto current_time = get_current_time();

		if (client_time <= current_time) {
			{
				auto scope = measure_scope(performance.receiving_messages);
				handle_server_messages();
			}

			const auto new_local_entropy = [&]() -> std::optional<mode_entropy> {
				if (is_gameplay_on()) {
					return total_collected.extract(
						get_controlled_character(), 
						get_local_player_id(), 
						make_accumulator_input(in)
					);
				}

				return std::nullopt;
			}();

			const bool in_game = new_local_entropy != std::nullopt;

			if (is_connected()) {
				auto scope = measure_scope(performance.sending_messages);

				send_pending_commands();

				if (in_game) {
					auto new_client_entropy = new_local_entropy->get_for(
						get_viewed_character(), 
						get_local_player_id()
					);

					send_to_server(new_client_entropy);
				}
			}

			{
				auto scope = measure_scope(performance.sending_packets);
				send_packets_if_its_time();
			}

			if (in_game) {
				auto referential_arena = get_arena_handle(client_arena_type::REFERENTIAL);
				auto predicted_arena = get_arena_handle(client_arena_type::PREDICTED);

				auto audiovisual_post_solve = callbacks.post_solve;

				auto for_each_effect_queue = [&](const const_logic_step step, auto callback) {
					namespace M = messages;

					auto& q = step.transient.messages;

					callback(q.get_queue<M::start_particle_effect>());
					callback(q.get_queue<M::stop_particle_effect>());

					callback(q.get_queue<M::start_sound_effect>());
					callback(q.get_queue<M::stop_sound_effect>());
					callback(q.get_queue<M::start_multi_sound_effect>());

					callback(q.get_queue<M::thunder_effect>());
					callback(q.get_queue<M::exploding_ring_effect>());
				};

				{
					auto scope = measure_scope(performance.unpacking_remote_steps);

					auto referential_post_solve = [&](const const_logic_step step) {
						audiovisual_post_solve_settings settings;

						if (is_spectating_referential()) {
							settings.prediction = prediction_input::offline();
						}
						else {
							const auto input = prediction_input::unpredictable_for(get_viewed_character());

							auto erase_predictable_messages = [&](auto& from_queue) {
								erase_if(
									from_queue,
									[&](const auto& m) {
										return !m.get_predictability().should_play(input);
									}
								);
							};

							for_each_effect_queue(step, erase_predictable_messages);
							settings.prediction = input;
						}

						{
							auto& notifications = step.get_queue<messages::game_notification>();

							const auto current_time = get_current_time();

							erase_if(notifications, [this, current_time](const auto& msg) {
								return client_gui.chat.add_entry_from_game_notification(current_time, msg, get_local_player_id());
							});
						}

						audiovisual_post_solve(step, settings);
					};

					auto referential_callbacks = solver_callbacks(
						default_solver_callback(),
						referential_post_solve,
						default_solver_callback()
					);

					auto advance_referential = [&](const auto& entropy) {
						referential_arena.advance(entropy, referential_callbacks, referential_solve_settings);

						const auto& added = entropy.general.added_player;

						if (logically_set(added)) {
							handle_new_session(added);
						}
					};

					auto advance_repredicted = [&](const auto& entropy) {
						/* 
							Note that we do not post-solve for the re-simulation process. 
							We only post-solve once for the predicted cosmos, when we actually move forward in time.
						*/

						const auto reprediction_result = predicted_arena.advance(
							entropy, 
							solver_callbacks(), 
							repredicted_solve_settings
						);

						schedule_reprediction_if_inconsistent(reprediction_result);
					};

					auto unpack = [&](const compact_server_step_entropy& entropy) {
						auto mode_id_to_entity_id = [&](const mode_player_id& mode_id) {
							return get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
								[&](const auto& typed_mode) {
									return typed_mode.lookup(mode_id);
								}
							);
						};

						auto get_settings_for = [&](const mode_player_id& mode_id) {
							return player_metas[mode_id.value].public_settings.character_input;
						};

						return entropy.unpack(mode_id_to_entity_id, get_settings_for);
					};

					const auto result = receiver.unpack_deterministic_steps(
						in.simulation_receiver,
						in.interp,
						in.past_infection,

						get_viewed_character(),

						unpack,

						referential_arena,
						predicted_arena,

						advance_referential,
						advance_repredicted
					);

					performance.accepted_commands.measure(result.total_accepted);

					if (result.malicious_server) {
						LOG("There was a problem unpacking steps from the server. Disconnecting.");
						log_malicious_server();
						disconnect();
					}

					if (result.desync && !now_resyncing) {
						pending_request = special_client_request::RESYNC;
						now_resyncing = true;

#if DUMP_BEFORE_AND_AFTER_ROUND_START
						const auto preffix = typesafe_sprintf("%x_desync%x_", augs::getpid(), referential_arena.get_round_num());

						referential_arena.on_mode(
							[&](const auto& mode) {
								::dump_for_debugging(
									lua,
									preffix,
									referential_arena.get_cosmos(),
									mode
								);
							}
						);
#endif
					}
				}

				{
					auto scope = measure_scope(performance.stepping_forward);

					{
						auto& p = receiver.predicted_entropies;

						const auto& max_commands = vars.max_predicted_client_commands;
						const auto num_commands = p.size();

						if (num_commands > max_commands) {
							set_disconnect_reason(typesafe_sprintf(
								"Number of predicted commands (%x) exceeded max_predicted_client_commands (%x).", 
								num_commands,
								max_commands
							));

							disconnect();
						}

						performance.predicted_steps.measure(num_commands);

						p.push_back(*new_local_entropy);
					}

					auto predicted_post_solve = [&](const const_logic_step step) {
						if (is_spectating_referential()) {
							return;
						}

						const auto input = prediction_input::predictable_for(get_viewed_character());

						auto erase_unpredictable_messages = [&](auto& from_queue) {
							erase_if(
								from_queue,
								[&](const auto& m) {
									return !m.get_predictability().should_play(input);
								}
							);
						};

						for_each_effect_queue(step, erase_unpredictable_messages);

						audiovisual_post_solve_settings settings;
						settings.prediction = input;

						audiovisual_post_solve(step, settings);
					};

					auto predicted_callbacks = solver_callbacks(
						default_solver_callback(),
						predicted_post_solve,
						default_solver_callback()
					);

#if USE_CLIENT_PREDICTION
					const auto forward_step_result = predicted_arena.advance(
						*new_local_entropy, 
						predicted_callbacks, 
						predicted_solve_settings
					);

					schedule_reprediction_if_inconsistent(forward_step_result);
#else
					(void)predicted_solve_settings;
					(void)predicted_callbacks;
					(void)callbacks;
#endif
				}
			}

			if (in_game) {
				client_time += get_inv_tickrate();
			}
			else {
				client_time += default_inv_tickrate;
			}

			update_stats(in.network_stats);
			total_collected.clear();
		}
	}

	template <class T>
	void control(const T& t) {
		total_collected.control(t);
	}

	void accept_game_gui_events(const game_gui_entropy_type&);

	augs::path_type get_unofficial_content_dir() const;

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void ensure_handler() {}

	mode_player_id get_local_player_id() const;
	std::optional<session_id_type> find_local_session_id() const;
	std::optional<session_id_type> find_session_id(mode_player_id) const;
	mode_player_id find_by(session_id_type) const;

	online_arena_handle<false> get_arena_handle(std::optional<client_arena_type> = std::nullopt);
	online_arena_handle<true> get_arena_handle(std::optional<client_arena_type> = std::nullopt) const;

	void log_malicious_server();

	bool is_connected() const;
	void disconnect();

	bool is_gameplay_on() const;
	setup_escape_result escape();

	void update_stats(network_info&) const;
	bool is_spectating_referential() const;

	bool is_loopback() const;

	const entropy_accumulator& get_entropy_accumulator() const {
		return total_collected;
	}

	bool handle_input_before_game(
		handle_input_before_game_input
	);

	void draw_custom_gui(const draw_setup_gui_input& in) const;

	template <class F>
	decltype(auto) on_mode_with_input(F&& callback) const {
		return get_arena_handle().on_mode_with_input(std::forward<F>(callback));
	}

	std::optional<arena_player_metas> get_new_player_metas();

	const arena_player_metas* find_player_metas() const {
		return std::addressof(player_metas);
	}

	auto get_current_requested_settings() const {
		return current_requested_settings;
	}
};
