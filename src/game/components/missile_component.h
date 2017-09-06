#pragma once
#include "augs/pad_bytes.h"

#include "augs/misc/timer.h"
#include "augs/misc/value_meter.h"
#include "augs/misc/stepped_timing.h"

#include "augs/graphics/rgba.h"

#include "game/assets/ids/sound_buffer_id.h"
#include "game/assets/ids/particle_effect_id.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

namespace components {
	struct missile {
		// GEN INTROSPECTOR struct components::missile
		meter_value_type damage_amount = 12;

		float impulse_upon_hit = 100.f;
		float impulse_multiplier_against_sentience = 1.f;

		bool damage_upon_collision = true;
		bool destroy_upon_damage = true;
		bool constrain_lifetime = true;
		bool damage_falloff = false;

		int damage_charges_before_destruction = 1;

		vec2 custom_impact_velocity;

		float damage_falloff_starting_distance = 500.f;
		float minimum_amount_after_falloff = 5.f;

		float max_lifetime_ms = 2000.f;
		float recoil_multiplier = 1.f;

		augs::stepped_timestamp when_released;
		augs::stepped_timestamp when_detonates;

		float homing_towards_hostile_strength = 0.f;
		entity_id particular_homing_target;
		
		child_entity_id trace_sound_entity;
		child_entity_id trace_particles_entity;

		sound_effect_input trace_sound;
		sound_effect_input destruction_sound;
		sound_effect_input pass_through_held_item_sound;

		particle_effect_input muzzle_leave_particles;
		particle_effect_input trace_particles;
		particle_effect_input destruction_particles;

		vec2 saved_point_of_impact_before_death;
		// END GEN INTROSPECTOR
	};
}