#pragma once
#include "game/assets/ids/asset_ids.h"
#include "game/enums/item_holding_stance.h"

#include "augs/misc/enum/enum_array.h"
#include "game/enums/weapon_action_type.h"

struct stance_action {
	// GEN INTROSPECTOR struct stance_action
	assets::torso_animation_id perform;
	assets::torso_animation_id returner;
	// END GEN INTROSPECTOR
};

struct stance_animations {
	// GEN INTROSPECTOR struct stance_animations
	assets::torso_animation_id carry;

	augs::enum_array<stance_action, weapon_action_type> actions;

	assets::torso_animation_id chambering;
	assets::torso_animation_id grip_to_mag;
	assets::torso_animation_id pocket_to_mag;
	assets::torso_animation_id arming_throwable;
	// END GEN INTROSPECTORS
};

namespace invariants {
	struct torso {
		// GEN INTROSPECTOR struct invariants::torso
		augs::enum_array<stance_animations, item_holding_stance> stances;

		assets::legs_animation_id forward_legs;
		assets::legs_animation_id strafe_legs;

		unsigned min_strafe_facing = 50;
		unsigned max_strafe_facing = 130;

		real32 strafe_face_interp_mult = 0.f;
		// END GEN INTROSPECTOR

		auto calc_leg_anim(
			const vec2 velocity,
			const real32 face_degrees
		) const; 

		template <class... Args>
		const stance_animations& calc_stance(Args&&... args) const;
	};
}

namespace components {

}
