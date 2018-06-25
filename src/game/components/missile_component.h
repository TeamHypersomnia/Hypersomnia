#pragma once
#include "augs/pad_bytes.h"

#include "augs/misc/constant_size_vector.h"
#include "augs/misc/value_meter.h"
#include "augs/misc/timing/stepped_timing.h"

#include "augs/math/vec2.h"

#include "augs/graphics/rgba.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/ids/asset_ids.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_flavour_id.h"

#include "game/detail/sentience_shake.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

namespace components {
	struct missile {
		// GEN INTROSPECTOR struct components::missile
		int damage_charges_before_destruction = 1;
		real32 power_multiplier_of_sender = 1.f;
		real32 initial_speed = 0.f;

		augs::stepped_timestamp when_fired;

		signi_entity_id particular_homing_target;
		
		vec2 saved_point_of_impact_before_death;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct missile {
		using remnant_flavour_id = constrained_entity_flavour_id<
			invariants::remnant
		>;

		// GEN INTROSPECTOR struct invariants::missile
		meter_value_type damage_amount = 12;

		float muzzle_velocity_mult = 1.f;

		float impulse_upon_hit = 10.f;
		float impulse_multiplier_against_sentience = 10.f;
		sentience_shake victim_shake = { 400.f, 1.f };

		bool damage_upon_collision = true;
		bool destroy_upon_damage = true;
		bool constrain_lifetime = true;
		bool damage_falloff = false;

		float damage_falloff_starting_distance = 500.f;
		float minimum_amount_after_falloff = 5.f;

		float max_lifetime_ms = 2000.f;
		float recoil_multiplier = 1.f;

		float homing_towards_hostile_strength = 0.f;

		sound_effect_input trace_sound;
		sound_effect_input destruction_sound;
		sound_effect_input ricochet_sound;
		sound_effect_input pass_through_held_item_sound;

		particle_effect_input ricochet_particles;
		particle_effect_input muzzle_leave_particles;
		bool trace_particles_fly_backwards = false;
		bool spawn_exploding_ring = true;
		pad_bytes<2> pad;
		particle_effect_input trace_particles;
		particle_effect_input destruction_particles;

		std::array<remnant_flavour_id, 4> remnant_flavours;
		// END GEN INTROSPECTOR
	};
}