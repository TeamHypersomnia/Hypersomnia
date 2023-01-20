#pragma once
#include "game/enums/marker_type.h"
#include "game/enums/faction_type.h"

namespace invariants {
	struct point_marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::point_marker
		point_marker_type type = point_marker_type::TEAM_SPAWN;
		// END GEN INTROSPECTOR
	};

	struct box_marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::box_marker
		area_marker_type type = area_marker_type::BOMBSITE_A;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct components::marker
		faction_type associated_faction = faction_type::METROPOLIS;
		pad_bytes<sizeof(item_flavour_id) + sizeof(int)> dummy_compat;
		// END GEN INTROSPECTOR
	};
}
