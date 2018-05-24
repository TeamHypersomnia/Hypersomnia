#pragma once
#include "game/assets/ids/asset_ids.h"
#include "game/enums/item_holding_stance.h"

#include "augs/misc/enum/enum_array.h"

struct stance_animations {
	// GEN INTROSPECTOR struct stance_animations
	assets::torso_animation_id carry;
	assets::torso_animation_id shoot;
	// END GEN INTROSPECTORS
};

namespace invariants {
	struct torso {
		// GEN INTROSPECTOR struct invariants::torso
		augs::enum_array<stance_animations, item_holding_stance> stances;
		stance_animations akimbo;

		assets::legs_animation_id forward_legs;
		assets::legs_animation_id strafe_legs;
		// END GEN INTROSPECTOR
	};
};

namespace components {

};
