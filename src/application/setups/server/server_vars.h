#pragma once
#include <string>
#include "augs/network/network_simulator_settings.h"
#include "augs/misc/constant_size_string.h"
#include "augs/network/network_types.h"
#include "augs/misc/constant_size_vector.h"
#include "application/network/address_and_port.h"

using arena_pool_type = augs::constant_size_vector<arena_identifier, max_arenas_in_pool_v, true>;

struct arena_switching_settings {
	// GEN INTROSPECTOR struct arena_switching_settings
	int switch_once_every_n_matches = 2;
	int vote_rounds_before = 0;
	// END GEN INTROSPECTOR
};

struct server_solvable_vars {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct server_solvable_vars
	arena_switching_settings arena_switching;

	arena_identifier current_arena = "";
	augs::constant_size_string<max_ruleset_name_length_v> override_default_ruleset = "";
	// END GEN INTROSPECTOR
};

struct server_vars {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct server_vars
	server_name_type server_name;
	address_and_port notified_server_list;

	bool allow_nat_traversal = true;

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
	uint32_t max_unauthorized_rcon_commands = 100;
	uint32_t max_bots = 0;
	float log_performance_once_every_secs = 1;
	float sleep_mult = 0.1f;
	// END GEN INTROSPECTOR
};

struct private_server_vars {
	// GEN INTROSPECTOR struct private_server_vars
	std::string master_rcon_password = "";
	std::string rcon_password = "";
	// END GEN INTROSPECTOR
};
