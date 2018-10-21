#pragma once
#include "augs/math/vec2.h" 
#include "game/components/transform_component.h"
#include "game/enums/melee_fighter_state.h"
#include "augs/pad_bytes.h"
#include "game/enums/weapon_action_type.h"

namespace invariants {
	struct melee_fighter {
		// GEN INTROSPECTOR struct invariants::melee_fighter
		real32 attack_speed_mult = 1.f;
		real32 base_fist_damage = 23.f;
		real32 damage_mult = 1.f;
		// END GEN INTROSPECTOR
	};
}

struct melee_flags {
	// GEN INTROSPECTOR struct melee_flags
	augs::enum_boolset<weapon_action_type> actions;
	bool block = false;
	pad_bytes<1> pad;
	// END GEN INTROSPECTOR
};

namespace components {
	struct melee_fighter {
		// GEN INTROSPECTOR struct components::melee_fighter
		melee_flags flags;
		melee_fighter_state state = melee_fighter_state::READY;
		weapon_action_type action = weapon_action_type::COUNT;
		// END GEN INTROSPECTOR

		void reset_flags() {
			flags = {};
		}
	};
}