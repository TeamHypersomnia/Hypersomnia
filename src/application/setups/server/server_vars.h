#pragma once
#include <string>
#include "augs/network/network_simulator_settings.h"
#include "augs/misc/constant_size_string.h"
#include "augs/network/network_types.h"
#include "augs/misc/constant_size_vector.h"
#include "application/network/address_and_port.h"
#include "application/setups/server/server_webhook_vars.h"
#include "augs/misc/secure_hash.h"
#include "application/arena/arena_playtesting_context.h"

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

struct server_vars {
	static constexpr bool force_read_field_by_field = true;

	bool operator==(const server_vars&) const = default;

	// GEN INTROSPECTOR struct server_vars
	arena_identifier arena;
	game_mode_name_type game_mode;

	std::optional<arena_playtesting_context> playtesting_context;

	server_name_type server_name;
	address_and_port notified_server_list;
	bool suppress_new_community_server_webhook = false;

	bool allow_nat_traversal = true;
	bool allow_direct_arena_file_downloads = true;
	address_string_type external_arena_files_provider = "";

	uint32_t send_heartbeat_to_server_list_once_every_secs = 10;
	uint32_t resolve_server_list_address_once_every_secs = 60;

	uint32_t resolve_internal_address_once_every_secs = 60 * 10;

	uint32_t move_to_spectators_if_afk_for_secs = 120;
	uint32_t kick_if_afk_for_secs = 7200;
	uint32_t kick_if_no_network_payloads_for_secs = 10;
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
};

struct server_private_vars {
	// GEN INTROSPECTOR struct server_private_vars
	std::string master_rcon_password = "";
	std::string rcon_password = "";

	std::string discord_webhook_url = "";
	std::string telegram_webhook_url = "";
	std::string telegram_channel_id = "@hypersomnia_monitor";
	// END GEN INTROSPECTOR
};
