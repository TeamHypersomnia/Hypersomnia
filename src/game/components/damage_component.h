#pragma once
#include "augs/math/vec2.h"

#include "game/assets/sound_buffer_id.h"
#include "game/assets/particle_effect_id.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/misc/timer.h"
#include "augs/graphics/pixel.h"
#include "augs/padding_byte.h"

#include "game/detail/sentience_meter.h"

namespace components {
	struct damage {
		// GEN INTROSPECTOR struct components::damage
		meter_value_type amount = 12;

		float impulse_upon_hit = 100.f;
		float impulse_multiplier_against_sentience = 1.f;

		entity_id sender;
		entity_id sender_capability;

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

		float current_lifetime_ms = 0.f;

		float homing_towards_hostile_strength = 0.f;
		entity_id particular_homing_target;
		
		child_entity_id trace_sound;

		sound_response bullet_trace_sound_response;
		sound_response destruction_sound_response;

		particle_effect_response muzzle_leave_particle_effect_response;
		particle_effect_response bullet_trace_particle_effect_response;
		particle_effect_response destruction_particle_effect_response;

		vec2 saved_point_of_impact_before_death;
		// END GEN INTROSPECTOR
	};
}