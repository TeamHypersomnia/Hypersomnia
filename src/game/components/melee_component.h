#pragma once
#include "augs/math/vec2.h" 
#include "augs/pad_bytes.h"

#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/enums/weapon_action_type.h"
#include "augs/misc/enum/enum_array.h"
#include "game/detail/damage/damage_definition.h"

#include "game/container_sizes.h"
#include "augs/pad_bytes.h"
#include "augs/math/physics_structs.h"

struct melee_clash_def {
	// GEN INTROSPECTOR struct melee_clash_def
	real32 impulse = 2000.f;

	sound_effect_input sound;
	particle_effect_input particles;
	// END GEN INTROSPECTOR
};

struct melee_attack_definition {
	// GEN INTROSPECTOR struct melee_attack_definition
	real32 cooldown_ms = 500.f;

	damage_definition damage;

	real32 obstacle_hit_recoil = 40.f;
	real32 sentience_hit_recoil = 10.f;

	real32 wielder_impulse = 20.f;
	real32 wielder_inert_for_ms = 250.f;
	real32 cp_required = 5.f;

	sound_effect_input init_sound;
	particle_effect_input init_particles;
	particle_effect_input wielder_init_particles;

	melee_clash_def clash;

	bool returning_animation_on_finish = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR
};

struct melee_throw_def {
	// GEN INTROSPECTOR struct melee_throw_def
	damage_definition damage;
	impulse_mults boomerang_impulse = { 2000.f, 1.5f };
	melee_clash_def clash;
	real32 min_speed_to_hurt = 50.f;
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct melee {
		// GEN INTROSPECTOR struct invariants::melee
		augs::enum_array<melee_attack_definition, weapon_action_type> actions;
		melee_throw_def throw_def;
		real32 movement_speed_bonus = 1.1f;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct melee {
		// GEN INTROSPECTOR struct components::melee
		augs::stepped_timestamp when_passed_held_item;
		augs::stepped_timestamp when_clashed;
		augs::stepped_timestamp when_inflicted_damage;
		// END GEN INTROSPECTOR
	};
}