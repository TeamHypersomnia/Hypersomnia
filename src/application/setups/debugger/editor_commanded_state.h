#pragma once
#include "application/setups/debugger/editor_view.h"
#include "application/intercosm.h"
#include "application/predefined_rulesets.h"

struct editor_commanded_state {
	// GEN INTROSPECTOR struct editor_commanded_state
	editor_view_ids view_ids;
	intercosm work;
	predefined_rulesets rulesets;
	// END GEN INTROSPECTOR
};
