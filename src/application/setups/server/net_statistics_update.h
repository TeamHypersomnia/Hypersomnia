#pragma once

struct net_statistics_entry {
	uint8_t ping;
	uint8_t download_progress;
};

struct net_statistics_update {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct net_statistics_update
	augs::constant_size_vector<net_statistics_entry, max_mode_players_v> stats;
	// END GEN INTROSPECTOR
};
