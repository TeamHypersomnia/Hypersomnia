#pragma once
#include <string>
#include "augs/network/network_simulator_settings.h"
#include "augs/filesystem/path_declaration.h"

struct client_jitter_vars {
	// GEN INTROSPECTOR struct client_jitter_vars
	uint32_t buffer_at_least_steps = 2;
	uint32_t buffer_at_least_ms = 20;
	uint8_t max_commands_to_squash_at_once = 255;
	// END GEN INTROSPECTOR
};

struct client_net_vars {
	// GEN INTROSPECTOR struct client_net_vars
	client_jitter_vars jitter;
	// END GEN INTROSPECTOR
};

struct client_chat_settings {
	// GEN INTROSPECTOR struct client_chat_settings
	unsigned chat_window_width = 300;
	vec2i chat_window_offset = vec2i(10, 300);

	unsigned show_recent_chat_messages_num = 5;
	float keep_recent_chat_messages_for_seconds = 8.f;
	float chat_messages_fade_last_seconds = 2.f;
	// END GEN INTROSPECTOR
};

struct client_vars {
	// GEN INTROSPECTOR struct client_vars
	std::string nickname = "Player";
	client_net_vars net;

	augs::maybe_network_simulator network_simulator;

	unsigned max_buffered_server_commands = 1000;
	unsigned max_predicted_client_commands = 3000u;

	bool spectate_referential_state = true;
	std::string rcon_password = "";
	client_chat_settings client_chat;
	augs::path_type avatar_image_path;
	// END GEN INTROSPECTOR
};
