#pragma once
#include <string>
#include "augs/network/network_simulator_settings.h"

struct client_jitter_vars {
	// GEN INTROSPECTOR struct client_jitter_vars
	uint32_t buffer_ms = 33;
	uint32_t merge_commands_when_above_ms = 50;
	uint8_t max_commands_to_squash_at_once = 255;
	// END GEN INTROSPECTOR
};

struct client_net_vars {
	// GEN INTROSPECTOR struct client_net_vars
	client_jitter_vars jitter;
	// END GEN INTROSPECTOR
};

struct client_vars {
	// GEN INTROSPECTOR struct client_vars
	std::string nickname = "Player";
	client_net_vars net;

	augs::maybe_network_simulator network_simulator;

	unsigned max_buffered_server_commands = 1000;
	unsigned max_predicted_client_commands = 255;
	// END GEN INTROSPECTOR
};
