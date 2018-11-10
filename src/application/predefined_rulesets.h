#pragma once
#include "game/modes/ruleset_id.h"
#include "game/modes/rulesets.h"

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
