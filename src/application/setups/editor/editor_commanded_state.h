#pragma once
#include "application/setups/editor/editor_view.h"
#include "game/modes/all_mode_includes.h"
#include "application/intercosm.h"

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

struct rulesets_meta {
	// GEN INTROSPECTOR struct rulesets_meta
	ruleset_id playtest_default;
	ruleset_id server_default;
	// END GEN INTROSPECTORS
};

struct predefined_rulesets {
	// GEN INTROSPECTOR struct predefined_rulesets
	all_rulesets_map all;
	rulesets_meta meta;
	// END GEN INTROSPECTOR
};

struct editor_commanded_state {
	// GEN INTROSPECTOR struct editor_commanded_state
	editor_view_ids view_ids;
	intercosm work;
	predefined_rulesets rulesets;
	// END GEN INTROSPECTOR
};
