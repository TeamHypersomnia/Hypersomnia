#pragma once
#include <string>

struct client_jitter_vars {
	// GEN INTROSPECTOR struct client_jitter_vars
	uint32_t buffer_ms = 33;
	uint32_t merge_commands_when_above_ms = 50;
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
	// END GEN INTROSPECTOR
};
