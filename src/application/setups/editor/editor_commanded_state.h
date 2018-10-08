#pragma once
#include "application/setups/editor/editor_view.h"
#include "game/modes/all_mode_includes.h"

struct intercosm;

struct editor_commanded_state {
	// GEN INTROSPECTOR struct editor_commanded_state
	editor_view_ids view_ids;
	intercosm work;
	all_mode_vars_maps mode_vars;
	// END GEN INTROSPECTOR
};
