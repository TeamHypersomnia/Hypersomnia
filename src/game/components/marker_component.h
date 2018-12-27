#pragma once
#include "game/enums/marker_type.h"
#include "game/enums/faction_type.h"

struct generator_marker {
	// GEN INTROSPECTOR struct generator_marker
	item_flavour_id generated_eq;
	int charges = 1;
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct point_marker {
		// GEN INTROSPECTOR struct invariants::point_marker
		point_marker_type type = point_marker_type::TEAM_SPAWN;
		// END GEN INTROSPECTOR
	};

	struct box_marker {
		// GEN INTROSPECTOR struct invariants::box_marker
		area_marker_type type = area_marker_type::BOMBSITE_A;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct marker {
		// GEN INTROSPECTOR struct components::marker
		faction_type associated_faction = faction_type::METROPOLIS;
		generator_marker generator;
		// END GEN INTROSPECTOR
	};
}
