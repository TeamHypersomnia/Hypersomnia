#pragma once
#include <array>
#include "augs/network/network_types.h"
#include "application/network/requested_client_settings.h"

struct steam_auth_request_payload {
	std::vector<std::byte> ticket_bytes;
};

struct arena_player_avatar_payload {
	std::vector<std::byte> image_bytes;
};

struct arena_player_network_stats {
	int ping = -1;
	uint8_t download_progress = 255;
};

struct synced_player_meta {
	public_client_settings public_settings;
	bool authenticated = false;
};

struct arena_player_meta {
	arena_player_avatar_payload avatar;
	arena_player_network_stats stats;

	synced_player_meta synced;

	void clear() {
		avatar.image_bytes.clear();

		stats = {};
		synced = {};
	}
};

using arena_player_metas = std::array<arena_player_meta, max_mode_players_v>;
