#pragma once
#include "application/setups/server/server_vars.h"

struct synced_dynamic_vars {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct synced_dynamic_vars
	bool run_ranked_logic = false;
	bool friendly_fire = true;
	server_ranked_vars ranked;
	// END GEN INTROSPECTOR

	bool operator==(const synced_dynamic_vars&) const = default;
};

