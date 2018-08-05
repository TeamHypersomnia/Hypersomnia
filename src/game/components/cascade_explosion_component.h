#pragma once
#include "game/cosmos/entity_id.h"
#include "game/enums/adverse_element_type.h"
#include "game/assets/ids/asset_ids.h"
#include "game/detail/explosions.h"

#include "augs/misc/timing/stepped_timing.h"
#include "augs/templates/variated.h"

namespace components {
	struct cascade_explosion {
		// GEN INTROSPECTOR struct components::cascade_explosion
		augs::stepped_timestamp when_next_explosion;
		unsigned explosions_left = 10;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct cascade_explosion {
		// GEN INTROSPECTOR struct invariants::cascade_explosion
		standard_explosion_input explosion;
		real32 explosion_scale_variation = 0.3f;

		augs::mult_variated<real32> explosion_interval_ms = { 100.f, 0.2f };
		real32 max_explosion_angle_displacement = 20.f;

		real32 circle_collider_radius = 100.f;
		// END GEN INTROSPECTOR
	};
}
