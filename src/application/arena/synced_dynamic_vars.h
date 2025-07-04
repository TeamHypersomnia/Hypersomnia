#pragma once
#include "application/setups/server/server_vars.h"

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
	augs::maybe<per_actual_faction<uint8_t>> bots_override;
	// END GEN INTROSPECTOR

	bool operator==(const synced_dynamic_vars&) const = default;

	bool is_ranked_server() const {
		return ranked.is_ranked_server();
	}
};

