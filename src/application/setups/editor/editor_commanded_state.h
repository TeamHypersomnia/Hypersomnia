#pragma once
#include "application/setups/editor/editor_view.h"
#include "game/modes/all_mode_includes.h"
#include "application/intercosm.h"

struct mode_vars_id {
	// GEN INTROSPECTOR struct mode_vars_id
	mode_type_id type_id;
	raw_mode_vars_id raw;
	// END GEN INTROSPECTOR
};

struct predefined_mode_vars {
	// GEN INTROSPECTOR struct predefined_mode_vars
	all_mode_vars_maps vars;
	mode_vars_id default_mode;
	// END GEN INTROSPECTOR
};

struct editor_commanded_state {
	// GEN INTROSPECTOR struct editor_commanded_state
	editor_view_ids view_ids;
	intercosm work;
	predefined_mode_vars mode_vars;
	// END GEN INTROSPECTOR
};
