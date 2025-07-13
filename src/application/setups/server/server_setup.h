#pragma once
#include <cstddef>
#include <cstdint>
#include <future>
#include "application/masterserver/netcode_address_hash.h"
#include "augs/math/camera_cone.h"
#include "game/detail/render_layer_filter.h"
#include "application/setups/server/server_listen_input.h"
#include "application/setups/server/server_assigned_teams.h"
#include "application/intercosm.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "application/setups/default_setup_settings.h"
#include "application/input/entropy_accumulator.h"

#include "application/setups/setup_common.h"
#include "game/modes/all_mode_includes.h"
#include "game/modes/mode_entropy.h"

#include "augs/network/network_types.h"
#include "application/setups/server/server_vars.h"
#include "application/setups/server/server_client_state.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/misc/serialization_buffers.h"

#include "application/network/server_step_entropy.h"
#if !HEADLESS
#include "application/gui/client/client_gui_state.h"
#include "view/mode_gui/arena/arena_gui_mixin.h"
#endif
#include "application/network/network_common.h"

#include "application/setups/server/chat_structs.h"
#include "application/setups/server/server_profiler.h"
#include "3rdparty/yojimbo/netcode/netcode.h"
#include "application/nat/nat_type.h"
#include "application/setups/server/server_nat_traversal.h"

#include "application/setups/server/rcon_level.h"
#include "game/messages/mode_notification.h"
#include "application/setups/server/file_chunk_packet.h"
#include "application/arena/synced_dynamic_vars.h"
#include "application/setups/server/server_temp_var_overrides.h"
#include "steam_rich_presence_pairs.h"

struct netcode_socket_t;
struct config_json_table;
struct draw_setup_gui_input;
struct synced_meta_update;

struct packaged_official_content;

namespace net_messages {
	struct client_welcome;
}

struct add_to_arena_input {
	client_id_type client_id;
	entity_name_str nickname;
};

class server_adapter;

struct resolve_address_result;
struct editor_project;

struct arena_files_database_entry {
	augs::path_type path;
	std::vector<std::byte> cached_file;

	void free_opened_file() {
		std::vector<std::byte>().swap(cached_file);
	}
};

using arena_files_database_type = std::unordered_map<augs::secure_hash_type, arena_files_database_entry>;

struct client_requested_chat;

class webrtc_server_detail;

class server_setup : 
	public default_setup_settings
#if !HEADLESS
	, public arena_gui_mixin<server_setup> /* For the admin player */
#endif
{
#if !HEADLESS
	using arena_gui_base = arena_gui_mixin<server_setup>;
#endif

	/* This is loaded from the arena folder */
	intercosm scene;
	cosmos_solvable_significant clean_round_state;

	all_rulesets_variant ruleset;

	/* Other replicated state */
	all_modes_variant current_mode_state;

	client_vars integrated_client_vars;

	server_vars canon_with_confd_vars;
	server_vars vars;
	server_private_vars private_vars;

	server_public_vars last_broadcast_public_vars;
	synced_dynamic_vars last_broadcast_dynamic_vars;

	server_runtime_info runtime_info;

	server_public_vars make_public_vars() const;
	synced_dynamic_vars make_synced_dynamic_vars() const;

	/* The rest is server-specific */
	std::shared_ptr<webrtc_server_detail> webrtc_server;

	std::size_t arena_cycle_current_index = 0;
	std::vector<std::size_t> shuffled_cycle_indices;
	randomization cycle_rng = randomization::from_random_device();

	augs::path_type current_arena_folder;
	augs::secure_hash_type current_arena_hash;

	const packaged_official_content& official;

	std::unique_ptr<editor_project> last_loaded_project;
	arena_files_database_type arena_files_database;

	augs::server_listen_input last_start;
	server_assigned_teams assigned_teams;

	std::optional<augs::dedicated_server_input> dedicated;

	std::unordered_set<augs::secure_hash_type> cached_currently_downloaded_files;
	std::unordered_set<augs::secure_hash_type> opened_arena_files;

	auto quit_playtesting_or(custom_imgui_result result) const {
		if (vars.playtesting_context && result == custom_imgui_result::GO_TO_MAIN_MENU) {
			return custom_imgui_result::QUIT_PLAYTESTING;
		}

		return result;
	}

	auto quit_playtesting_or(setup_escape_result result) const {
		if (vars.playtesting_context && result == setup_escape_result::GO_TO_MAIN_MENU) {
			return setup_escape_result::QUIT_PLAYTESTING;
		}

		return result;
	}

	augs::serialization_buffers buffers;

	entropy_accumulator local_collected;
	std::vector<mode_player_id> moved_to_spectators;

	compact_server_step_entropy step_collected;
	bool reinference_necessary = false;

	augs::propagate_const<std::unique_ptr<server_adapter>> server;
	std::array<server_client_state, max_incoming_connections_v> clients;
	uint32_t next_session_id = 0;

	server_client_state integrated_client;

	unsigned ticks_until_sending_packets = 0;
	unsigned ticks_until_sending_hash = 0;
	net_time_t when_last_sent_net_statistics = 0;
	net_time_t when_last_sent_admin_public_settings = 0;
	net_time_t when_last_sent_heartbeat_to_server_list = 0;
	net_time_t when_last_sent_tell_me_my_address = 0;
	net_time_t when_last_resolved_server_list_addr = 0;
	net_time_t when_last_resolved_internal_address = 0;

	net_time_t when_last_used_map_command = 0;
	net_time_t when_last_changed_map_due_to_idle = 0;

	net_time_t dont_check_timeouts_until = 0;

	double tell_me_my_address_stamp = 0;

	static bool is_webrtc_only();

	bool handle_auxiliary_command(const netcode_address_t& from, const std::byte* packet, int n);
    bool send_packet_override(const netcode_address_t&,const std::byte*,int);
    int receive_packet_override(netcode_address_t&,std::byte*,int);

	std::chrono::system_clock::time_point when_to_check_for_updates;
	hour_and_minute_str when_to_check_for_updates_last_var;

	std::vector<std::byte> heartbeat_buffer;

#if BUILD_NATIVE_SOCKETS
	std::future<resolve_address_result> future_resolved_server_list_addr;

	std::future<std::optional<netcode_address_t>> future_internal_address;
	std::optional<netcode_address_t> internal_address;

	std::optional<netcode_address_t> resolved_server_list_addr;
	std::optional<netcode_address_t> external_address;
#endif

	net_time_t server_time = 0.0;
	bool shutdown_scheduled = false;
	bool request_restart_after_shutdown = false;

	bool rebuild_player_meta_viewables = false;
	arena_player_metas integrated_player_metas;

#if !HEADLESS
	client_gui_state integrated_client_gui;
#endif
	std::string failure_reason;

#if BUILD_NATIVE_SOCKETS
	std::optional<server_nat_traversal> nat_traversal;
#endif
	bool suppress_community_server_webhook_this_run = false;
	std::string name_suffix;
	server_temp_var_overrides overrides;

	enum class job_type {
		AVATAR,
		AUTH,

		NOTIFICATION,
		REPORT_MATCH
	};


#if PLATFORM_WEB
	webrtc_id_type resolved_this_server_webrtc_id;
#else
	uint32_t duel_pic_counter = 0;

	struct webhook_job {
		mode_player_id player_id;
		std::optional<server_client_session_id> client_session_id;

		job_type type = job_type::NOTIFICATION;

		std::unique_ptr<std::future<std::string>> job;
	};

	std::vector<webhook_job> pending_jobs;
#endif

	template <class F>
	void push_session_webhook_job(mode_player_id player_id, job_type type, F&& f);

	template <class F>
	void push_notification_job(F&& f);

	template <class F>
	void push_avatar_job(mode_player_id player_id, F&& f);

	template <class F>
	void push_auth_job(mode_player_id player_id, F&& f);

	void request_auth(mode_player_id player_id, const auth_request_payload&);

	void finalize_webhook_jobs();

	void push_connected_webhook(mode_player_id);
	void push_duel_of_honor_webhook(const std::string& first, const std::string& second);
	void push_match_summary_webhook(const messages::match_summary_message& summary);
	void push_duel_interrupted_webhook(const messages::duel_interrupted_message& summary);

	void push_report_match_webhook(const messages::match_summary_message& summary);

	std::string get_next_duel_pic_link();

	void check_for_updates();
	bool check_for_updates_once = false;
	bool write_vars_to_disk_once = false;

	std::size_t num_suspended_players() const;

	bool is_currently_suspended(std::string account_id) const;
	mode_player_id find_suspended_player_id(std::string account_id) const;

public:
	net_time_t last_logged_at = 0;
	server_profiler profiler;

	bool should_check_for_updates_once();
	bool should_write_vars_to_disk_once();
private:
	/* No server state follows later in code. */

	static net_time_t get_current_time();

	template <class H, class S>
	static decltype(auto) get_arena_handle_impl(S& self) {
		return H {
			self.current_mode_state,
			self.scene,
			self.scene.world,
			self.ruleset,
			self.clean_round_state,
			self.last_broadcast_dynamic_vars
		};
	}

	void handle_client_messages();
	void advance_clients_state();

	void rebroadcast_player_synced_metas();
	void rebroadcast_synced_dynamic_vars();
	void send_server_step_entropies(const compact_server_step_entropy& total);
	void broadcast_net_statistics();

	void send_full_arena_snapshot_to(const client_id_type);
	void send_complete_solvable_state_to(const client_id_type);

	void refresh_available_direct_download_bandwidths();
	void send_packets_if_its_time();

	void send_tell_me_my_address();
	void send_tell_me_my_address_if_its_time();

	void send_heartbeat_to_server_list();
	void send_heartbeat_to_server_list_if_its_time();

	void resolve_server_list();
	void resolve_heartbeat_host_if_its_time();
	void resolve_internal_address_if_its_time();

	void request_immediate_heartbeat();

	void perform_automoves_to_spectators();
	void accept_entropy_of_client(
		const mode_player_id,
		const total_client_entropy&
	);

	friend server_adapter;

	template <class T, class F>
	message_handler_result handle_payload(
		const client_id_type&, 
		F&& read_payload
	);

	template <class P>
	message_handler_result handle_rcon_payload(
		const client_id_type&, 
		rcon_level_type,
		const P& payload
	);

	void init_client(const client_id_type&);
	void unset_client(const client_id_type&);

	yojimbo::Address get_client_address(const client_id_type&) const;
	mode_player_id get_integrated_player_id() const;
	client_id_type get_integrated_client_id() const;

	void reinfer_if_necessary_for(const compact_server_step_entropy& entropy);
	bool server_list_enabled() const;
	bool has_sent_any_heartbeats() const;
	void shutdown();

	bool handle_masterserver_response(
		const netcode_address_t& from,
		const std::byte* packet_buffer,
		const std::size_t packet_bytes
	);

	bool handle_gameserver_command(
		const netcode_address_t& from,
		const std::byte* packet_buffer,
		const std::size_t packet_bytes
	);

	void start_ranked_match_if_conditions_met();

	template <class T>
	void choose_next_map_from(const T&);
	void handle_changing_maps_on_idle();

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool has_additional_highlights = false;

	server_setup(
		const packaged_official_content& official,
		const augs::server_listen_input&,
		const server_vars&,
		const server_vars& canon_with_confd_vars,
		const server_private_vars&,
		const client_vars& integrated_client_vars,
		std::optional<augs::dedicated_server_input>,

#if BUILD_NATIVE_SOCKETS
		const std::optional<server_nat_traversal_input> nat_traversal_input,
#endif
		bool suppress_community_server_webhook_this_run,
		const server_assigned_teams& assigned_teams,
		const std::string& webrtc_signalling_server_url,
		const std::string& name_suffix = "",
		const server_temp_var_overrides& initial_overrides = server_temp_var_overrides()
	);

	~server_setup();

	bool server_restart_requested() const {
		return request_restart_after_shutdown;
	}

	static mode_player_id to_mode_player_id(const client_id_type&);
	static client_id_type to_client_id(const mode_player_id&);

	std::optional<server_client_session_id> find_session_id(const client_id_type&) const;
	std::optional<server_client_session_id> find_session_id(const mode_player_id&) const;

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	auto get_interpolation_ratio() const {
		const auto dt = get_viewed_cosmos().get_fixed_delta().in_seconds<double>();
		return std::min(1.0, (get_current_time() - server_time) / dt);
	}

	entity_id get_controlled_character_id() const;

#if HEADLESS
	auto get_viewed_character_id() const {
		return entity_id();
	}
#endif

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return scene.viewables;
	}

	void customize_for_viewing(config_json_table&) const;

	void apply(const config_json_table&);
	bool apply(const server_vars&, bool first_time = false);
	void apply(const server_private_vars&);

	void try_apply(const public_client_settings& integrated_client_requested_settings);

	void rechoose_arena();
	void rebroadcast_server_public_vars();

	std::string describe_client(const client_id_type id) const;
	void log_malicious_client(const client_id_type id);

	double get_audiovisual_speed() const;
	double get_inv_tickrate() const;

	template <class C>
	void advance(
		const server_advance_input& in,
		const C& callbacks
	) {
		if (!is_running()) {
			return;
		}

#if BUILD_NATIVE_SOCKETS
		if (nat_traversal) {
			nat_traversal->last_detected_nat = in.last_detected_nat;

			if (auto socket = find_underlying_socket()) {
				nat_traversal->send_packets(*socket);
			}

			nat_traversal->advance();
		}
#endif

		const auto current_time = get_current_time();

		while (server_time <= current_time) {
			if (shutdown_scheduled) {
				shutdown();
				shutdown_scheduled = false;
				return;
			}

			auto scope = measure_scope(profiler.step);

			finalize_webhook_jobs();
			check_for_updates();

			handle_changing_maps_on_idle();

			step_collected.clear();

			{
				auto scope = measure_scope(profiler.advance_adapter);
				handle_client_messages();
			}

			{
				auto scope = measure_scope(profiler.advance_clients_state);
				advance_clients_state();
			}

			{
				/* 
					Extract entropy from the built-in server player. 
					If it is a dedicated server, this will only extract the general commands,
					like player added or removed.
				*/

				{
					const auto admin_entropy = local_collected.assemble_for(
						get_viewed_character(), 
						get_integrated_player_id(),
						in.make_accumulator_input()
					);

					if (!admin_entropy.empty()) {
						step_collected += { get_integrated_player_id(), admin_entropy };
						reset_afk_timer();
					}
				}

				step_collected.general += local_collected.mode_general;
				perform_automoves_to_spectators();

				local_collected.clear();
			}

			{
				auto scope = measure_scope(profiler.send_entropies);

				rebroadcast_player_synced_metas();
				rebroadcast_synced_dynamic_vars();
				send_server_step_entropies(step_collected);
				broadcast_net_statistics();
			}

			{
				auto scope = measure_scope(profiler.send_packets);
				send_packets_if_its_time();

				resolve_internal_address_if_its_time();
				resolve_heartbeat_host_if_its_time();
				send_heartbeat_to_server_list_if_its_time();
				send_tell_me_my_address_if_its_time();
			}

			reinfer_if_necessary_for(step_collected);

			{
				auto scope = measure_scope(profiler.solve_simulation);

				const auto unpacked = unpack(step_collected);
				const auto arena = get_arena_handle();

				if (is_dedicated()) {
					auto post_solve = [&](auto old_callback, const const_logic_step step) {
						handle_abandon_requests(step);
						ban_players_who_left_for_good(step);
						lock_ranked_roster_if_started(step);

						{
							auto& notifications = step.get_queue<messages::mode_notification>();

							if (!notifications.empty()) {
								request_immediate_heartbeat();
							}
						}

						default_server_post_solve(step);
						old_callback(step);
					};

					auto new_callbacks = callbacks.combine(
						default_solver_callback(),
						post_solve,
						default_solver_callback()
					);

					arena.advance(
						unpacked, 
						new_callbacks, 
						solve_settings()
					);
				}
				else {
					auto post_solve = [&](auto old_callback, const const_logic_step step) {
						auto& notifications = step.get_queue<messages::mode_notification>();

						if (!notifications.empty()) {
							request_immediate_heartbeat();
						}

#if !HEADLESS
						const auto current_time = get_current_time();

						erase_if(notifications, [this, current_time](const auto& msg) {
							return integrated_client_gui.chat.add_entry_from_mode_notification(current_time, msg, get_local_player_id());
						});
#endif

						default_server_post_solve(step);
						old_callback(step);
					};

					auto new_callbacks = callbacks.combine(
						default_solver_callback(),
						post_solve,
						default_solver_callback()
					);

					arena.advance(
						unpacked, 
						new_callbacks, 
						solve_settings()
					);

					const auto& removed = unpacked.general.removed_player;

					if (logically_set(removed)) {
						reset_player_meta_to_default(removed);
					}
				}
			}

			const auto advanced_dt = get_inv_tickrate();
			server_time += advanced_dt;

			if (is_integrated()) {
				/*
					Don't let us fall back too much.
					If we didn't do this, performance drops might make the game freeze further
					because we'll increasing the simulation debt.

					This also fixes the freeze on web when e.g. the tab is changed.
				*/

				server_time = std::max(server_time, current_time - advanced_dt * 5);
			}

			update_stats(in.server_stats);
			step_collected.clear();
		}

		refresh_available_direct_download_bandwidths();
		clean_unused_cached_files();

		log_performance();
	}

	template <class T>
	void control(const T& t) {
		local_collected.control(t);
	}

	void accept_game_gui_events(const game_gui_entropy_type&);

	augs::path_type get_unofficial_content_dir() const;

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void ensure_handler() {}

	online_arena_handle<false> get_arena_handle();
	online_arena_handle<true> get_arena_handle() const;

	void disconnect_and_unset(const client_id_type&);

	void kick(const client_id_type&, const std::string& reason);
	message_handler_result abort_or_kick_if_debug(const client_id_type&, const std::string& reason);
	void ban(const client_id_type&, const std::string& reason);

	mode_player_id get_local_player_id() const {
		return get_integrated_player_id();
	}

	faction_type get_assigned_faction(mode_player_id) const;
	faction_type get_assigned_faction() const;

	bool is_gameplay_on() const;

#if !HEADLESS
	custom_imgui_result perform_custom_imgui(perform_custom_imgui_input);
	setup_escape_result escape();
	void draw_custom_gui(const draw_setup_gui_input& in) const;
	bool requires_cursor() const;
	bool handle_input_before_game(
		const handle_input_before_game_input in
	);
#endif

	bool is_running() const;
	bool should_have_admin_character() const;

	void sleep_until_next_tick();

	void update_stats(server_network_info&) const;

	server_step_entropy unpack(const compact_server_step_entropy&) const;

	rcon_level_type get_rcon_level(const client_id_type&) const;

	const entropy_accumulator& get_entropy_accumulator() const {
		return local_collected;
	}

	template <class F>
	decltype(auto) on_mode_with_input(F&& callback) const {
		return get_arena_handle().on_mode_with_input(std::forward<F>(callback));
	}

	void broadcast(const ::server_broadcasted_chat&, std::optional<client_id_type> except = std::nullopt);
	void broadcast_info(const std::string&, chat_target_type = chat_target_type::INFO_CRITICAL);

	std::string find_client_nickname(const client_id_type&) const;

	auto get_game_gui_subject_id() const {
		return get_viewed_character_id();
	}

	const arena_player_metas* find_player_metas() const;
	std::optional<arena_player_metas> get_new_player_metas();

	std::nullopt_t get_new_ad_hoc_images() {
		return std::nullopt;
	}

	enum class for_each_flag {
		ONLY_CONNECTED,
		WITH_INTEGRATED,

		COUNT
	};

	using for_each_flags = augs::enum_boolset<for_each_flag>;

	template <class T, class F>
	static void for_each_id_and_client_impl(T& self, F&& callback, for_each_flags);

	template <class F>
	void for_each_id_and_client(F&& callback, for_each_flags = {});

	template <class F>
	void for_each_id_and_client(F&& callback, for_each_flags = {}) const;

	bool is_integrated() const;
	bool is_dedicated() const;

	void reset_player_meta_to_default(const mode_player_id&);
	void log_performance();

	::synced_meta_update make_synced_meta_update_from(
		const server_client_state&,
		const client_id_type& id
	) const;

	bool player_added_to_mode(mode_player_id) const;

	const netcode_socket_t* find_underlying_socket() const;

	uint32_t get_num_slots() const;
	uint32_t get_num_connected() const;
	uint32_t get_num_bots() const;

	void after_all_drawcalls(game_frame_buffer&) {}
	void do_game_main_thread_synced_op(renderer_backend_result&) {}

	void reset_afk_timer();

	server_client_state& get_client_state(mode_player_id);
	const server_client_state& get_client_state(mode_player_id) const;

	const server_client_state* find_client_state(const std::string& nickname) const;

	server_client_state* find_client_state(mode_player_id);
	const server_client_state* find_client_state(mode_player_id) const;
	mode_player_id find_client_by_account_id(const std::string&) const;

	std::vector<std::string> get_all_nicknames() const;

	server_name_type get_server_name() const;
	std::string get_current_arena_name() const;
	game_mode_name_type get_current_game_mode_name() const;

	void default_server_post_solve(const const_logic_step step);
	void ban_players_who_left_for_good(const const_logic_step step);
	void handle_abandon_requests(const const_logic_step step);
	void lock_ranked_roster_if_started(const const_logic_step step);

	void log_match_end_json(const messages::match_summary_message&);
	void log_match_start_json(const messages::team_match_start_message&);

	void schedule_shutdown();
	void schedule_restart();
	void send_goodbye_to_masterserver();

	void broadcast_shutdown_message();

	void rescan_runtime_prefs();
	void rescan_arenas_on_disk();
	void broadcast_runtime_info_for_rcon();
	void refresh_runtime_info_for_rcon();

	void set_client_is_downloading_files(client_id_type, server_client_state& c, downloading_type);

	file_chunk_index_type calc_num_chunks_per_tick_per_downloader() const;

	bool send_file_chunk(client_id_type id, const arena_files_database_entry& entry, file_chunk_index_type i);
	void clean_unused_cached_files();

	void apply_nonzoomedout_visible_world_area(vec2);

	const server_vars& get_current_vars() const {
		return vars;
	}

	bool is_playtesting_server() const;

	std::string get_steam_join_command_line() const;
	void get_steam_rich_presence_pairs(steam_rich_presence_pairs&) const;

	bool is_joinable() const;
	bool is_ranked_live() const;
	bool is_ranked_live_or_starting() const;
	bool has_suspended_players() const;

	bool is_connection_request_packet(
		const netcode_address_t& from,
		const std::byte* packet_buffer,
		const std::size_t packet_bytes
	) const;

	void restart_match();

	void handle_client_chat_command(client_id_type, const ::client_requested_chat&);

	bool is_ranked_server() const;

	bool can_use_map_command_now() const;

	void choose_next_map_from_cycle();
	bool is_idle() const;

	bool has_assigned_teams() const;
	faction_type get_assigned_team(const std::string&) const;

	std::string get_browser_location() const;
	std::string get_connect_string() const;

	void do_integrated_rcon_gui(bool force = false);

	static void send_custom_webhook(const custom_webhook_data& webhook, const std::string& message);
};
