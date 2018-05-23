#pragma once
#include "game/assets/ids/asset_ids.h"

namespace invariants {
	struct torso {
		// GEN INTROSPECTOR struct invariants::torso
		assets::torso_animation_id bare_walk;

		assets::torso_animation_id rifle_carry;
		assets::torso_animation_id rifle_shoot;

		assets::torso_animation_id pistol_carry;
		assets::torso_animation_id pistol_shoot;

		assets::torso_animation_id akimbo_carry;
		assets::torso_animation_id akimbo_shoot;

		assets::legs_animation_id forward_legs;
		assets::legs_animation_id strafe_legs;
		// END GEN INTROSPECTOR
	};
};

namespace components {

};
