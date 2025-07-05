#pragma once
#include "application/setups/server/server_vars.h"

struct bots_request {
	// GEN INTROSPECTOR struct bots_request
	mode_player_id requester;
	uint8_t first = -1;
	uint8_t second = -1;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return first != uint8_t(-1);
	}

	bool operator==(const bots_request&) const = default;
};

struct synced_dynamic_vars {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct synced_dynamic_vars
	bool preassigned_factions = false;
	bool all_assigned_present = false;
	bool all_authenticated = false;
	bool all_not_banned = false;
	bool friendly_fire = true;
	bool force_short_match = false;
	server_ranked_vars ranked;
	bots_request bots_override;
	// END GEN INTROSPECTOR

	bool operator==(const synced_dynamic_vars&) const = default;

	bool is_ranked_server() const {
		return ranked.is_ranked_server();
	}
};

