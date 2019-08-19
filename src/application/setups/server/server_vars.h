#pragma once
#include <string>
#include "augs/network/network_simulator_settings.h"

struct server_vars {
	// GEN INTROSPECTOR struct server_vars
	std::string current_arena = "";
	std::string override_default_ruleset = "";

	unsigned kick_if_no_messages_for_secs = 60;
	unsigned kick_if_away_from_keyboard_for_secs = 240;
	unsigned time_limit_to_enter_game_since_connection = 10;

	unsigned reset_resync_timer_once_every_secs = 10;
	unsigned max_client_resyncs = 3;

	unsigned send_updates_once_every_tick = 1;

	unsigned max_buffered_client_commands = 1000;

	unsigned state_hash_once_every_tick = 1;
	float send_net_statistics_update_once_every_secs = 1;

	float max_kick_ban_linger_secs = 2;

	augs::maybe_network_simulator network_simulator;

	bool auto_authorize_loopback_for_rcon = true;
	unsigned max_unauthorized_rcon_commands = 100;
	unsigned max_bots = 0;
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
