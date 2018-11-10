#pragma once
#include "game/modes/all_mode_includes.h"

using raw_ruleset_id = unsigned;

struct ruleset_id {
	// GEN INTROSPECTOR struct ruleset_id
	mode_type_id type_id;
	raw_ruleset_id raw;
	// END GEN INTROSPECTOR

	bool operator==(const ruleset_id& b) const {
		return type_id == b.type_id && raw == b.raw;
	}

	bool operator!=(const ruleset_id& b) const {
		return !operator==(b);
	}
};

