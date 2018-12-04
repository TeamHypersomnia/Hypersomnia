#pragma once
#include "game/modes/all_mode_includes.h"

struct mode_and_rules {
	// GEN INTROSPECTOR struct mode_and_rules
	all_modes_variant state = bomb_mode();
	raw_ruleset_id rules_id = raw_ruleset_id();
	// END GEN INTROSPECTOR

	void choose(const ruleset_id&);
};
