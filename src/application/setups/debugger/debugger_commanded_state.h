#pragma once
#include "application/setups/debugger/debugger_view.h"
#include "application/intercosm.h"
#include "application/predefined_rulesets.h"

struct debugger_commanded_state {
	// GEN INTROSPECTOR struct debugger_commanded_state
	debugger_view_ids view_ids;
	intercosm work;
	predefined_rulesets rulesets;
	// END GEN INTROSPECTOR
};
