#pragma once
#include "augs/math/simple_physics.h"
#include "game/assets/ids/asset_ids.h"

namespace invariants {
	struct head {
		// GEN INTROSPECTOR struct invariants::head
		assets::image_id head_image;
		assets::image_id shooting_head_image;
		real32 shake_rotation_damping = 1.f;
		real32 impulse_mult_on_shake = 1.f;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct head {
		// GEN INTROSPECTOR struct components::head
		real32 shake_rotation_amount = 0.f;
		// END GEN INTROSPECTOR
	};
}
