#pragma once
#include <cstdint>
#include <string>
#include "augs/network/network_simulator_settings.h"
#include "augs/filesystem/path_declaration.h"
#include "augs/graphics/rgba.h"
#include "augs/misc/constant_size_string.h"
#include "application/setups/client/demo_paths.h"
#include "view/client_arena_type.h"

using client_nickname_type = augs::constant_size_string<max_nickname_length_v>;
using clan_name_type = augs::constant_size_string<max_clan_length_v>;

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

	bool operator==(const client_chat_settings& b) const = default;
};

struct override_holder {
	mutable client_nickname_type nickname = "";
	bool operator==(const override_holder&) const { return true; };
};

struct client_vars {
	// GEN INTROSPECTOR struct client_vars
	bool use_account_nickname = true;
	bool use_account_avatar = true;

	client_nickname_type nickname = "";
	clan_name_type clan = "";

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
	bool record_demo = true;

	float max_direct_file_bandwidth = 2.0f;
	// END GEN INTROSPECTOR

	override_holder signed_in;

	client_nickname_type signed_in_nickname() const {
		return signed_in.nickname;
	}

	client_nickname_type get_nickname() const {
		if (!signed_in_nickname().empty()) {
			return signed_in_nickname();
		}

		return nickname;
	}

	bool operator==(const client_vars& b) const = default;
};
