#pragma once
#include "game/detail/sentience/pe_absorption.h"

namespace components {

}

/* TODO: Make a trivial variant out of this. */

namespace invariants {
	struct tool {
		// GEN INTROSPECTOR struct invariants::tool
		real32 defusing_speed_mult = 1.f;
		real32 pe_regeneration_mult = 1.f;
		pe_absorption_info pe_absorption;
		real32 spell_cost_amortization = 0.f;
		// END GEN INTROSPECTOR
	};
}