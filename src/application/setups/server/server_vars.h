#pragma once
#include <cstdint>
#include <string>
#include "augs/network/network_simulator_settings.h"
#include "augs/misc/constant_size_string.h"
#include "augs/network/network_types.h"
#include "augs/misc/constant_size_vector.h"
#include "application/network/host_with_default_port.h"
#include "application/setups/server/server_webhook_vars.h"
#include "augs/misc/secure_hash.h"
#include "application/arena/arena_playtesting_context.h"
#include "game/modes/difficulty_type.h"

using arena_pool_type = augs::constant_size_vector<arena_identifier, max_arenas_in_pool_v, true>;

struct arena_switching_settings {
	bool operator==(const arena_switching_settings&) const = default;

	// GEN INTROSPECTOR struct arena_switching_settings
	int switch_once_every_n_matches = 2;
	int vote_rounds_before = 0;
	// END GEN INTROSPECTOR
};

struct server_runtime_info {
	// GEN INTROSPECTOR struct server_runtime_info
	std::vector<arena_identifier> arenas_on_disk;
	std::string runtime_prefs;
	// END GEN INTROSPECTOR
};

struct server_public_vars {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct server_public_vars
	arena_identifier arena;
	game_mode_name_type game_mode;

	augs::secure_hash_type required_arena_hash = augs::secure_hash_type();
	std::optional<arena_playtesting_context> playtesting_context;

	address_string_type external_arena_files_provider;
	// END GEN INTROSPECTOR

	bool operator==(const server_public_vars&) const = default;
};

enum class ranked_autostart_type {
	// GEN INTROSPECTOR enum class ranked_autostart_type
	NEVER,
	TEAMS_EQUAL,
	SERVER_FULL,
	ALWAYS,
	COUNT
	// END GEN INTROSPECTOR
};

enum class arena_cycle_type {
	// GEN INTROSPECTOR enum class arena_cycle_type
	REPEAT_CURRENT,
	LIST,
	ALL_ON_DISK,

	COUNT
	// END GEN INTROSPECTOR
};

struct server_ranked_vars {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct server_ranked_vars
	ranked_autostart_type autostart_when = ranked_autostart_type::NEVER;
	uint16_t countdown_time = 40;
	uint16_t rejoin_time_limit = 150;
	uint8_t max_rejoins = 1;
	uint8_t match_unfreezes_in_secs = 5;
	bool freeze_match_on_disconnect = true;
	float client_network_timeout_secs = 1.0f;
	// END GEN INTROSPECTOR

	bool operator==(const server_ranked_vars&) const = default;

	uint16_t get_rejoin_time_limit(bool is_short) const {
		if (is_short) {
			return rejoin_time_limit / 2;
		}

		return rejoin_time_limit;
	}

	bool is_ranked_server() const {
		return autostart_when != ranked_autostart_type::NEVER;
	}
};

struct server_vars {
	static constexpr bool force_read_field_by_field = true;

	bool operator==(const server_vars&) const = default;

	// GEN INTROSPECTOR struct server_vars
	arena_identifier arena;
	game_mode_name_type game_mode;

	server_ranked_vars ranked;

	bool friendly_fire = true;
	bool bots = true;
	difficulty_type bot_difficulty = difficulty_type::EASY;

	arena_cycle_type cycle = arena_cycle_type::REPEAT_CURRENT;
	std::vector<arena_and_mode_identifier> cycle_list;
	game_mode_name_type cycle_always_game_mode;
	bool cycle_randomize_order = false;

	std::optional<arena_playtesting_context> playtesting_context;

	bool allow_webrtc_clients = true;

	bool webrtc_udp_mux = true;
	port_type webrtc_port_range_begin = 9000;
	port_type webrtc_port_range_end = 9050;

	server_name_type server_name;
	address_string_type notified_server_list;
	bool suppress_new_community_server_webhook = false;
	bool show_on_server_list = true;

	bool allow_nat_traversal = true;
	bool allow_direct_arena_file_downloads = true;
	address_string_type external_arena_files_provider = "";

	int autoupdate_delay = 0;

	bool daily_autoupdate = false;
	hour_and_minute_str daily_autoupdate_hour = "";

	float when_idle_change_maps_once_every_mins = 15.0f;
	uint32_t send_heartbeat_to_server_list_once_every_secs = 10;
	uint32_t resolve_server_list_address_once_every_secs = 60;

	uint32_t resolve_internal_address_once_every_secs = 60 * 10;

	uint32_t move_to_spectators_if_afk_for_secs = 120;
	uint32_t kick_if_afk_for_secs = 7200;
	float web_client_network_timeout_secs = 1.5f;
	float client_network_timeout_secs = 3.0f;
	bool authenticate_with_nicknames = false;
	float kick_if_unauthenticated_for_secs = 3.0f;
	uint32_t time_limit_to_enter_game_since_connection = 10;

	uint32_t reset_resync_timer_once_every_secs = 10;
	uint32_t max_client_resyncs = 3;

	uint32_t send_packets_once_every_tick = 1;

	uint32_t max_buffered_client_commands = 1000;

	uint32_t state_hash_once_every_tick = 1;
	float send_net_statistics_update_once_every_secs = 1;

	float max_kick_ban_linger_secs = 2;

	augs::maybe_network_simulator network_simulator;

	bool auto_authorize_loopback_for_rcon = true;
	bool auto_authorize_internal_for_rcon = false;
	uint32_t max_unauthorized_rcon_commands = 100;
	uint32_t max_bots = 0;
	float log_performance_once_every_secs = 1;
	float sleep_mult = 0.1f;

	float max_direct_file_bandwidth = 2.0f;

	server_webhook_vars webhooks;

	bool shutdown_after_first_match = false;

	bool sync_all_external_arenas_on_startup = false;
	// END GEN INTROSPECTOR

	float get_client_network_timeout_secs(const bool is_web) const {
		if (ranked.is_ranked_server()) {
			return ranked.client_network_timeout_secs;
		}

		if (is_web) {
			return web_client_network_timeout_secs;
		}

		return client_network_timeout_secs;
	}

	bool requires_authentication() const {
		return ranked.is_ranked_server() && kick_if_unauthenticated_for_secs > 0;
	}
};

struct url_and_key_pair {
	// GEN INTROSPECTOR struct url_and_key_pair
	std::string url;
	std::string header_apikey;
	// END GEN INTROSPECTOR

	bool operator==(const url_and_key_pair& b) const = default;
};

struct custom_webhook_data {
	// GEN INTROSPECTOR struct custom_webhook_data
	std::string url;
	std::string header_authorization;
	std::string clan;
	// END GEN INTROSPECTOR

	bool operator==(const custom_webhook_data& b) const = default;
};

struct server_private_vars {
	// GEN INTROSPECTOR struct server_private_vars
	std::string master_rcon_password = "";
	std::string rcon_password = "";

	std::string discord_webhook_url = "";
	std::string telegram_webhook_url = "";
	std::string telegram_channel_id = "@hypersomnia_monitor";

	std::string telegram_alerts_channel_id = "@hypersomnia_server_logs";
	std::string telegram_alerts_webhook_url;

	std::vector<custom_webhook_data> custom_webhook_urls;

	std::string steam_web_api_key = "";

	std::string report_ranked_match_api_key = "";
	std::string report_ranked_match_url = "https://hypersomnia.xyz/report_match";

	std::vector<url_and_key_pair> report_ranked_match_aux_endpoints;

	std::string check_ban_url = "https://hypersomnia.xyz/check_ban";
	// END GEN INTROSPECTOR

	bool operator==(const server_private_vars& b) const = default;
};
