#pragma once
#include "augs/math/vec2.h" 
#include "game/components/transform_component.h"
#include "game/enums/melee_fighter_state.h"
#include "augs/pad_bytes.h"
#include "game/enums/weapon_action_type.h"
#include "game/assets/animation.h"

using already_hit_entities = augs::constant_size_vector<signi_entity_id, 4>;

namespace invariants {
	struct melee_fighter {
		// GEN INTROSPECTOR struct invariants::melee_fighter
		real32 cooldown_speed_mult = 1.f;
		real32 base_fist_damage = 23.f;
		real32 damage_mult = 1.f;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct melee_fighter {
		// GEN INTROSPECTOR struct components::melee_fighter
		melee_fighter_state state = melee_fighter_state::READY;
		weapon_action_type action = weapon_action_type::COUNT;
		simple_animation_state anim_state;
		already_hit_entities hit_obstacles;
		already_hit_entities hit_others;
		transformr previous_frame_transform;
		vec2 first_separating_impulse;
		vec2 overridden_crosshair_base_offset;
		// END GEN INTROSPECTOR

		bool now_returning() const;
		bool fighter_orientation_frozen() const;
	};
}