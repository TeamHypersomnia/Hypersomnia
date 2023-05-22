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

	struct area_marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::area_marker
		area_marker_type type = area_marker_type::BOMBSITE;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct components::marker
		faction_type faction = faction_type::METROPOLIS;
		marker_letter_type letter = marker_letter_type::A;
		// END GEN INTROSPECTOR
	};
}
