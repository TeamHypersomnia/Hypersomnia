#pragma once
#include <cstdint>
#include <array>
#include "augs/network/network_types.h"
#include "application/network/requested_client_settings.h"
#include "application/main/auth_provider_type.h"

struct arena_player_avatar_payload {
	std::vector<std::byte> image_bytes;
};

struct arena_player_network_stats {
	int ping = -1;
	uint8_t download_progress = 255;
};

/*
	badge is decided by the server - shown in front of the nickname in the chat, and in place of the clan in the scoreboard.
	See server_setup::calc_badge.
*/

struct synced_player_meta {
	public_client_settings public_settings;
	uint8_t platform_type = 0;
	clan_name_type badge;
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
