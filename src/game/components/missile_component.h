#pragma once
#include "augs/pad_bytes.h"

#include "augs/misc/constant_size_vector.h"
#include "augs/misc/value_meter.h"
#include "augs/misc/timing/stepped_timing.h"

#include "augs/math/vec2.h"

#include "augs/graphics/rgba.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/ids/asset_ids.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_flavour_id.h"

#include "game/detail/sentience_shake.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

#include "game/detail/damage/damage_definition.h"

using remnant_flavour_id = constrained_entity_flavour_id<invariants::remnant>;
using remnant_flavour_vector = augs::constant_size_vector<remnant_flavour_id, 4>;

namespace components {
	struct missile {
		// GEN INTROSPECTOR struct components::missile
		int damage_charges_before_destruction = 1;
		real32 power_multiplier_of_sender = 1.f;
		real32 headshot_multiplier_of_sender = 1.f;
		real32 head_radius_multiplier_of_sender = 1.f;
		real32 initial_speed = 0.f;

		augs::stepped_timestamp when_last_ricocheted;
		augs::stepped_timestamp when_fired;

		signi_entity_id particular_homing_target;
		
		transformr saved_point_of_impact_before_death;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct missile {
		// GEN INTROSPECTOR struct invariants::missile
		damage_definition damage;

		real32 muzzle_velocity_mult = 1.f;

		bool damage_upon_collision = true;
		bool destroy_upon_damage = true;
		bool constrain_lifetime = true;
		bool damage_falloff = false;

		real32 damage_falloff_starting_distance = 500.f;
		real32 minimum_amount_after_falloff = 5.f;

		real32 max_lifetime_ms = 2000.f;
		real32 recoil_multiplier = 1.f;

		real32 homing_towards_hostile_strength = 0.f;
		real32 pe_damage_ratio = 0.25f;

		sound_effect_input trace_sound;
		sound_effect_input ricochet_sound;

		particle_effect_input ricochet_particles;
		particle_effect_input muzzle_leave_particles;
		bool trace_particles_fly_backwards = false;
		bool trace_sound_audible_to_shooter = false;
		pad_bytes<2> pad;
		particle_effect_input trace_particles;

		remnant_flavour_vector remnant_flavours;
		real32 ricochet_cooldown_ms = 24.f;
		real32 ricochet_born_cooldown_ms = 24.f;
		// END GEN INTROSPECTOR
	};
}