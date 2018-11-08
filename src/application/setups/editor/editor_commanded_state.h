#pragma once
#include "application/setups/editor/editor_view.h"
#include "game/modes/all_mode_includes.h"
#include "application/intercosm.h"

struct mode_rules_id {
	// GEN INTROSPECTOR struct mode_rules_id
	mode_type_id type_id;
	raw_mode_rules_id raw;
	// END GEN INTROSPECTOR
};

struct predefined_mode_rules {
	// GEN INTROSPECTOR struct predefined_mode_rules
	all_mode_rules_maps vars;
	mode_rules_id default_mode;
	// END GEN INTROSPECTOR
};

struct editor_commanded_state {
	// GEN INTROSPECTOR struct editor_commanded_state
	editor_view_ids view_ids;
	intercosm work;
	predefined_mode_rules mode_rules;
	// END GEN INTROSPECTOR
};
