#pragma once
#include <array>
#include "augs/network/network_types.h"

struct arena_player_avatar_payload {
	std::vector<std::byte> png_bytes;
};

struct arena_player_network_stats {
	int ping = -1;
};

struct arena_player_meta {
	arena_player_avatar_payload avatar;
	arena_player_network_stats stats;

	void clear() {
		avatar.png_bytes.clear();
	}
};

using arena_player_metas = std::array<arena_player_meta, max_incoming_connections_v>;
