#pragma once
#include <string>
#include "augs/network/network_simulator_settings.h"

struct server_vars {
	// GEN INTROSPECTOR struct server_vars
	std::string current_arena = "";
	std::string override_default_ruleset = "";

	std::string admin_nickname = "GameMaster";

	unsigned kick_if_inactive_for_secs = 60;
	unsigned time_limit_to_enter_game_since_connection = 10;

	unsigned send_updates_once_every_tick = 1;

	unsigned max_buffered_client_commands = 1000;

	unsigned state_hash_once_every_tick = 1;

	augs::maybe_network_simulator network_simulator;
	// END GEN INTROSPECTOR
};
