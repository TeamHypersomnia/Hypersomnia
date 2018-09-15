#pragma once
#include "game/assets/ids/asset_ids.h"
#include "game/enums/item_holding_stance.h"

#include "augs/misc/enum/enum_array.h"

struct stance_animations {
	// GEN INTROSPECTOR struct stance_animations
	assets::torso_animation_id carry;
	assets::torso_animation_id shoot;

	assets::torso_animation_id grip_to_mag;
	assets::torso_animation_id pocket_to_mag;
	// END GEN INTROSPECTORS

	const assets::torso_animation_id& get_chambering() const;
};

namespace invariants {
	struct torso {
		// GEN INTROSPECTOR struct invariants::torso
		augs::enum_array<stance_animations, item_holding_stance> stances;

		assets::legs_animation_id forward_legs;
		assets::legs_animation_id strafe_legs;

		unsigned min_strafe_facing = 50;
		unsigned max_strafe_facing = 130;

		float strafe_face_interp_mult = 0.f;
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
