#pragma once
#include "game/enums/marker_type.h"
#include "game/enums/faction_type.h"

struct marker_meta {
	// GEN INTROSPECTOR struct marker_meta
	faction_type associated_faction = faction_type::METROPOLIS;
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct point_marker {
		// GEN INTROSPECTOR struct invariants::point_marker
		point_marker_type type = point_marker_type::TEAM_SPAWN;
		marker_meta meta;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct box_marker {
		// GEN INTROSPECTOR struct invariants::box_marker
		area_marker_type type = area_marker_type::BOMBSITE_A;
		marker_meta meta;
		// END GEN INTROSPECTOR
	};
}
