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

		unsigned min_strafe_facing = 50;
		unsigned max_strafe_facing = 130;
		// END GEN INTROSPECTOR
	};
};

namespace components {

};

inline auto calc_leg_anim(
	const invariants::torso& torso,
	const vec2 velocity,
	const real32 face_degrees
) {
	const auto facing = velocity.degrees_between(vec2::from_degrees(face_degrees));

	if (facing >= torso.min_strafe_facing && facing <= torso.max_strafe_facing) {
		return torso.strafe_legs;
	}
	else {
		return torso.forward_legs;
	}
}
