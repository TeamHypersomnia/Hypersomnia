#pragma once
#include <string>

struct server_vars {
	// GEN INTROSPECTOR struct server_vars
	std::string current_arena = "test";
	std::string override_default_ruleset = "";

	unsigned kick_if_inactive_for_secs = 60;
	unsigned time_limit_to_enter_game_since_connection = 10;

	unsigned send_updates_once_every_tick = 1;
	// END GEN INTROSPECTOR
};
