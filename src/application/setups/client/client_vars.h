#pragma once
#include <string>
#include "augs/network/network_simulator_settings.h"
#include "augs/filesystem/path_declaration.h"
#include "augs/graphics/rgba.h"
#include "augs/misc/constant_size_string.h"
#include "application/setups/client/demo_paths.h"
#include "view/client_arena_type.h"

using client_nickname_type = augs::constant_size_string<max_nickname_length_v>;

struct client_jitter_vars {
	// GEN INTROSPECTOR struct client_jitter_vars
	uint32_t buffer_at_least_steps = 2;
	uint32_t buffer_at_least_ms = 20;
	uint8_t max_commands_to_squash_at_once = 255;
	// END GEN INTROSPECTOR

	bool operator==(const client_jitter_vars&) const = default;
};

struct client_net_vars {
	// GEN INTROSPECTOR struct client_net_vars
	client_jitter_vars jitter;
	// END GEN INTROSPECTOR

	bool operator==(const client_net_vars&) const = default;
};

struct client_chat_settings {
	// GEN INTROSPECTOR struct client_chat_settings
	unsigned chat_window_width = 300;
	vec2i chat_window_offset = vec2i(10, 300);

	rgba background_color = white;
	rgba border_color = white;

	unsigned show_recent_chat_messages_num = 5;
	float keep_recent_chat_messages_for_seconds = 8.f;
	float chat_messages_fade_last_seconds = 2.f;
	// END GEN INTROSPECTOR
};

struct client_vars {
	// GEN INTROSPECTOR struct client_vars
	client_nickname_type nickname = "Player";
	client_net_vars net;

	bool suppress_webhooks = false;

	augs::maybe_network_simulator network_simulator;

	unsigned max_buffered_server_commands = 1000;
	unsigned max_predicted_client_commands = 3000u;

	unsigned flush_demo_to_disk_once_every_secs = 10u;

	client_arena_type spectated_arena_type = client_arena_type::REFERENTIAL;
	std::string rcon_password = "";
	client_chat_settings client_chat;
	augs::path_type avatar_image_path;
	augs::maybe<std::string> demo_recording_path = augs::maybe<std::string>::enabled(DEMOS_DIR.string());

	float max_direct_file_bandwidth = 2.0f;
	// END GEN INTROSPECTOR
};
