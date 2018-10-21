#pragma once
#include "augs/math/vec2.h" 
#include "augs/pad_bytes.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/enums/weapon_action_type.h"
#include "augs/misc/enum/enum_array.h"

#include "game/container_sizes.h"
#include "augs/pad_bytes.h"

struct damaging_object_info {
	// GEN INTROSPECTOR struct damaging_object_info
	real32 base = 23.f;
	sentience_shake shake = { 400.f, 1.f };

	real32 impulse = 1000.f;
	real32 impulse_multiplier_against_sentience = 10.f;

	sound_effect_input impact_sound;
	particle_effect_input impact_particles;

	sound_effect_input pass_through_held_item_sound;
	// END GEN INTROSPECTOR
};

struct melee_clash_def {
	// GEN INTROSPECTOR struct melee_clash_def
	sound_effect_input sound;
	particle_effect_input particles;
	// END GEN INTROSPECTOR
};

struct melee_attack_definition {
	// GEN INTROSPECTOR struct melee_attack_definition
	damaging_object_info damage;

	real32 obstacle_hit_recoil = 40.f;
	real32 sentience_hit_recoil = 10.f;

	real32 wielder_impulse = 20.f;
	real32 cp_required = 5.f;

	sound_effect_input init_sound;
	particle_effect_input init_particles;

	melee_clash_def clash;

	bool reverse_animation_on_finish = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR
};

struct melee_throw_def {
	// GEN INTROSPECTOR struct melee_throw_def
	damaging_object_info damage;
	real32 boomerang_impulse = 2000.f;
	real32 min_velocity_to_hurt = 50.f;
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct melee {
		// GEN INTROSPECTOR struct invariants::melee
		augs::enum_array<melee_attack_definition, weapon_action_type> actions;
		melee_throw_def throw_def;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct melee {
		// GEN INTROSPECTOR struct components::melee
		real32 reserved = 0.f;
		// END GEN INTROSPECTOR
	};
}