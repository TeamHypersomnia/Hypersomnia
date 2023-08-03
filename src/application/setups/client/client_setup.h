#pragma once
#include <future>
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

#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/misc/serialization_buffers.h"

#include "view/mode_gui/arena/arena_gui_mixin.h"
#include "view/audiovisual_state/audiovisual_post_solve_settings.h"

#include "application/network/client_state_type.h"
#include "application/network/requested_client_settings.h"

#include "application/network/simulation_receiver.h"
#include "application/session_profiler.h"
#include "application/setups/client/lag_compensation_settings.h"

#include "view/client_arena_type.h"
#include "application/network/special_client_request.h"
#include "application/gui/client/rcon_gui.h"
#include "application/gui/client/chat_gui.h"
#include "application/gui/client/client_gui_state.h"
#include "view/mode_gui/arena/arena_player_meta.h"
#include "augs/texture_atlas/loaded_images_vector.h"
#include "application/setups/client/demo_file.h"
#include "application/setups/client/demo_step.h"
#include "application/gui/client/demo_player_gui.h"
#include "application/setups/client/client_demo_player.h"
#include "application/nat/nat_detection_settings.h"
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "application/setups/client/arena_downloading_session.h"
#include "application/setups/client/direct_file_download.h"
#include "application/setups/client/bandwidth_monitor.h"

struct config_lua_table;

class https_file_downloader;

constexpr double default_inv_tickrate = 1 / 60.0;

class client_adapter;

struct netcode_socket_t;
struct packaged_official_content;

struct arena_download_input {
	std::string arena_name;
	augs::secure_hash_type project_hash;
};

class client_setup : 
	public default_setup_settings,
	public arena_gui_mixin<client_setup>
{
	using arena_gui_base = arena_gui_mixin<client_setup>;
	friend arena_gui_base;
	friend client_adapter;

	/* This is loaded from the arena folder */
	intercosm scene;
	cosmos_solvable_significant clean_round_state;

	all_rulesets_variant ruleset;

	/* Other replicated state */
	all_modes_variant current_mode_state;
	server_public_vars sv_public_vars;
	augs::path_type current_arena_folder;

	mode_player_id client_player_id;

	cosmos predicted_cosmos;
	all_modes_variant predicted_mode;

	std::vector<special_client_request> pending_requests;
	bool now_resyncing = false;

	arena_player_metas player_metas;

	/* The rest is client-specific */
	sol::state& lua;
	const packaged_official_content& official;

	simulation_receiver receiver;

	address_and_port last_addr;
	std::string displayed_connecting_server_name;

	netcode_address_t resolved_server_address;
	client_state_type state = client_state_type::NETCODE_NEGOTIATING_CONNECTION;

	client_vars vars;
	requested_client_settings requested_settings;
	requested_client_settings current_requested_settings;

	entropy_accumulator total_collected;
	augs::serialization_buffers buffers;

	augs::propagate_const<std::unique_ptr<client_adapter>> adapter;
	net_time_t client_time = 0.0;
	net_time_t when_initiated_connection = 0.0;
	net_time_t when_sent_client_settings = -1;
	net_time_t when_sent_nat_punch_request = -1;
	net_time_t when_sent_last_keepalive = 0;

	std::string last_disconnect_reason;
	bool print_only_disconnect_reason = false;

	bool rebuild_player_meta_viewables = false;
	bool has_sent_avatar = false;
	client_gui_state client_gui;

	arena_download_input last_download_request;

	std::optional<arena_downloading_session> downloading;
	bool pause_solvable_stream = false;

	std::unique_ptr<https_file_downloader> external_downloader;
	std::optional<direct_file_download> direct_downloader;
	std::optional<augs::secure_hash_type> last_requested_direct_file_hash;
	uint32_t num_skip_chunks = 0;
	std::vector<file_chunk_packet> buffered_chunk_packets;
	BandwidthMonitor direct_bandwidth;

	using untimely_payload_variant = std::variant<arena_player_avatar_payload>;

	struct untimely_payload {
		session_id_type associated_id;
		untimely_payload_variant payload;
	};

	std::vector<untimely_payload> untimely_payloads;

	net_time_t when_last_flushed_demo = 0.0;
	augs::path_type recorded_demo_path;
	demo_step_num_type recorded_demo_step = 0;
	std::size_t written_messages = 0;

	std::vector<demo_step> unflushed_demo_steps;
	std::vector<demo_step> demo_steps_being_flushed;
	std::future<void> future_flushed_demo;
	bool was_demo_meta_written = false;

	client_demo_player demo_player;
	/* No client state follows later in code. */

	bool is_trying_external_download() const {
		return external_downloader != nullptr;
	}

	template <class U>
	bool handle_untimely(U&, session_id_type);

	bool push_or_handle(untimely_payload&);
	void handle_new_session(const add_player_input& in);

	template <class T>
	void set_demo_failed_reason(T&& reason) {
		last_disconnect_reason = typesafe_sprintf("Error during demo replay:\n%x", reason);
		print_only_disconnect_reason = true;
	}

	template <class T>
	void set_disconnect_reason(T&& reason, bool print_only_reason = false) {
		last_disconnect_reason = std::forward<T>(reason);
		LOG(last_disconnect_reason);
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
				self.ruleset,
				self.clean_round_state
			};
		}
		else {
			ensure_eq(t, client_arena_type::REFERENTIAL);

			return H {
				self.current_mode_state,
				self.scene,
				self.scene.world,
				self.ruleset,
				self.clean_round_state
			};
		}
	}

	void handle_incoming_payloads();
	void send_pending_commands();
	void send_packets();
	void exchange_file_packets();
	void traverse_nat_if_required();

	template <class T, class F>
	message_handler_result handle_payload(
		F&& read_payload
	);

	template <class T>
	void demo_record_server_message(T& message);

	void send_to_server(total_client_entropy&);

	client_arena_type get_viewed_arena_type() const;

	auto get_controlled_character() const {
		return get_viewed_cosmos()[get_controlled_character_id()];
	}

	void play_demo_from(const augs::path_type&);
	void record_demo_to(const augs::path_type&);

	void demo_replay_server_messages_from(const demo_step&);

	auto make_accumulator_input(const client_advance_input& in) {
		auto accumulator_in = in.make_accumulator_input();
		accumulator_in.settings.character = current_requested_settings.public_settings.character_input;
		return accumulator_in;
	}

	auto get_total_local_player_entropy(const client_advance_input& in) {
		const auto assembled = total_collected.assemble(
			get_controlled_character(), 
			get_local_player_id(), 
			make_accumulator_input(in)
		);

		total_collected.clear();

		return assembled;
	}

	void advance_demo_recorder();

	template <class Callbacks, class ServerPayloadProvider, class TotalLocalEntropyProvider>
	void advance_single_step(
		const client_advance_input& in,
		const Callbacks& callbacks,
		ServerPayloadProvider server_payload_provider,
		TotalLocalEntropyProvider local_entropy_provider
	) {
		if (is_recording()) {
			unflushed_demo_steps.emplace_back();
		}

		auto scope = augs::scope_guard([this]() {
			if (is_recording()) {
				advance_demo_recorder();
			}
		});

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

		{
			auto scope = measure_scope(performance.receiving_messages);
			server_payload_provider();
		}

		const auto new_local_entropy = [&]() -> std::optional<mode_entropy> {
			if (is_gameplay_on()) {
				const auto total = local_entropy_provider();

				if (is_recording()) {
					auto& recorded_step = get_currently_recorded_step();
					recorded_step.local_entropy.emplace(total);
				}

				return total;
			}

			return std::nullopt;
		}();

		const bool in_game = new_local_entropy.has_value();

		if (is_connected()) {
			auto scope = measure_scope(performance.sending_messages);

			send_pending_commands();

			if (in_game) {
				auto new_client_entropy = new_local_entropy->get_for(
					get_viewed_character(), 
					get_local_player_id()
				);

				//LOG("In game. Sending client entropy. Predicted %x: ", receiver.predicted_entropies.size());
				send_to_server(new_client_entropy);
			}
		}

		{
			auto scope = measure_scope(performance.sending_packets);
			send_packets();
			traverse_nat_if_required();
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
						auto& notifications = step.get_queue<messages::mode_notification>();

						const auto current_time = get_current_time();

						erase_if(notifications, [this, current_time](const auto& msg) {
							return client_gui.chat.add_entry_from_mode_notification(current_time, msg, get_local_player_id());
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
					set_disconnect_reason("There was a problem unpacking steps from the server. Disconnecting.");
					disconnect();
				}

				if (result.desync && !now_resyncing) {
					special_request(special_client_request::RESYNC_ARENA);
					now_resyncing = true;
				}
			}

			{
				auto scope = measure_scope(performance.stepping_forward);

				{
					auto& p = receiver.predicted_entropies;

					const auto& max_commands = vars.max_predicted_client_commands;
					const auto num_commands = p.size();

					if (!is_replaying()) {
						if (num_commands > max_commands) {
							set_disconnect_reason(typesafe_sprintf(
								"Number of predicted commands (%x) exceeded max_predicted_client_commands (%x).", 
								num_commands,
								max_commands
							));

							disconnect();
						}
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

	void perform_demo_player_imgui(augs::window& window);
	void snap_interpolation_of_viewed();

	void request_direct_file_download(const augs::secure_hash_type&);

	bool setup_external_arena_download_session();
	void setup_direct_arena_download_session();
	bool start_downloading_session();

	void advance_external_downloader();

	void send_download_progress();
	bool send_keepalive_download_progress();

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool has_additional_highlights = false;

	client_setup(
		sol::state& lua,
		const packaged_official_content& official,
		const client_start_input&,
		const client_vars& initial_vars,
		const nat_detection_settings& nat_detection,
		port_type preferred_binding_port
	);

	~client_setup();

	const cosmos& get_viewed_cosmos() const;

	auto get_interpolation_ratio() const {
		if (pause_solvable_stream) {
			return 0.0;
		}

		const auto dt_secs = get_viewed_cosmos().get_fixed_delta().in_seconds<double>();

		if (is_replaying()) {
			return demo_player.timer.next_step_progress_fraction(dt_secs);
		}

		/* 
			0 = step is exactly a delta away (current = client_time - dt_secs)
			1 = step should happen now       (current = client_time)

			Will never be less than 0 because client_time is only ever incremented by dt_secs 
				conditional upon that it is LESS than current_time

			if current = client_time then
			ratio = (client_time - (client_time - dt_secs)) / dt_secs = 1
		*/

		const auto at_0 = client_time - dt_secs;
		return (get_current_time() - at_0) / dt_secs;
	}

	const_entity_handle get_viewed_character() const {
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

	template <class Callbacks>
	void advance(
		const client_advance_input& in,
		const Callbacks& callbacks
	) {
		if (is_replaying()) {
			auto advance_with = [&](const demo_step& step) {
				const auto dt = get_inv_tickrate();

				auto local_entropy_provider = [&]() {
					return step.local_entropy ? *step.local_entropy : mode_entropy();
				};

				advance_single_step(in, callbacks, [&](){ demo_replay_server_messages_from(step); }, local_entropy_provider);
				return dt;
			};

			bool needs_snap = false;

			auto seeking_advance = [&](const demo_step& step) {
				const auto dt = get_inv_tickrate();

				auto local_entropy_provider = [&]() {
					return step.local_entropy ? *step.local_entropy : mode_entropy();
				};

				advance_single_step(in, solver_callbacks(), [&](){ demo_replay_server_messages_from(step); }, local_entropy_provider);
				needs_snap = true;
				return dt;
			};

			auto rewind = [&]() {
				client_demo_player player_backup = std::move(demo_player);

				{
					auto& l = lua;

					client_start_input client_in;
					client_in.replay_demo = player_backup.source_path;
					const auto vars_backup = vars;

					std::destroy_at(this);
					new (this) client_setup(l, official, client_in, vars_backup, nat_detection_settings(), port_type(0));
				}

				demo_player = std::move(player_backup);
			};

			demo_player.advance(
				in.frame_delta,
				advance_with,
				seeking_advance,
				rewind,
				get_inv_tickrate()
			);

			if (needs_snap) {
				snap_interpolation_of_viewed();
			}

			return;
		}

		const auto current_time = get_current_time();

		if (client_time < current_time) {
			if (downloading) {
				if (is_trying_external_download()) {
					if (send_keepalive_download_progress()) {
						handle_incoming_payloads();
						send_packets();
					}

					advance_external_downloader();
				}
				else {
					exchange_file_packets();
				}

				update_stats(in.network_stats);
			}
			else {
				auto local_entropy_provider = [&]() {
					return get_total_local_player_entropy(in);
				};

				advance_single_step(
					in, 
					callbacks, 
					[this](){ handle_incoming_payloads(); }, 
					local_entropy_provider
				);
			}
		}
	}

	template <class T>
	void control(const T& t) {
		total_collected.control(t);
	}

	void reset_afk_timer();

	void accept_game_gui_events(const game_gui_entropy_type&);

	augs::path_type get_unofficial_content_dir() const;

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void ensure_handler();

	mode_player_id get_local_player_id() const;
	std::optional<session_id_type> find_local_session_id() const;
	std::optional<session_id_type> find_session_id(mode_player_id) const;
	mode_player_id find_by(session_id_type) const;

	online_arena_handle<false> get_arena_handle(std::optional<client_arena_type> = std::nullopt);
	online_arena_handle<true> get_arena_handle(std::optional<client_arena_type> = std::nullopt) const;

	bool is_connected() const;
	void disconnect();

	bool is_gameplay_on() const;
	setup_escape_result escape();

	void update_stats(network_info&) const;
	bool is_spectating_referential() const;

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

	std::nullopt_t get_new_ad_hoc_images() {
		return std::nullopt;
	}

	const arena_player_metas* find_player_metas() const {
		return std::addressof(player_metas);
	}

	auto get_current_requested_settings() const {
		return current_requested_settings;
	}

	rcon_level_type get_rcon_level() const {
		return client_gui.rcon.level;
	}

	bool requires_cursor() const;
	bool is_replaying() const;
	bool is_paused() const;
	bool is_recording() const;
	demo_step& get_currently_recorded_step();
	void flush_demo_steps();
	void wait_for_demo_flush();
	bool demo_flushing_finished() const;

	template <class... Args>
	bool send_payload(Args&&... args);

	void perform_chat_input_bar();

	void after_all_drawcalls(game_frame_buffer&) {}
	void do_game_main_thread_synced_op(renderer_backend_result&) {}

	bool start_downloading_arena(const arena_download_input&);

	message_handler_result advance_downloading_session(augs::cptr_memory_stream next_received_file);

	bool finalize_arena_download();

	void special_request(special_client_request);
	bool try_load_arena_according_to(const server_public_vars&, bool allow_download);

	std::string get_displayed_connecting_server_name() const {
		if (displayed_connecting_server_name.empty()) {
			return "the game server";
		}

		return displayed_connecting_server_name;
	}

	auto get_current_file_download_progress() const;
	float get_current_file_percent_complete() const;
	float get_total_download_percent_complete(const bool smooth) const;

	bool handle_auxiliary_command(std::byte*, int n);

	void handle_received(const file_chunk_packet& chunk);
	file_chunk_index_type calc_num_chunks_per_tick() const;
};
