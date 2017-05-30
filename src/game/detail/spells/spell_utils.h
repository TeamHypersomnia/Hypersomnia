#pragma once
#include "game/components/transform_component.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"

template <class T>
void ignite_cast_sparkles(
	const T& spell_data,
	const logic_step step, 
	const components::transform caster_transform,
	const entity_id subject
) {
	particles_existence_input burst;
	burst.effect = spell_data.common.cast_sparkles;

	burst.create_particle_effect_entity(
		step,
		caster_transform,
		subject
	).add_standard_components(step);
}

template <class T>
void ignite_charging_particles(
	const T& spell_data,
	const logic_step step, 
	const components::transform caster_transform,
	const entity_id subject,
	const rgba col
) {
	particles_existence_input burst;
	burst.effect = spell_data.charging_particles;
	burst.effect.modifier.colorize = col;
	burst.effect.modifier.homing_target = subject;

	burst.create_particle_effect_entity(
		step,
		caster_transform,
		subject
	).add_standard_components(step);
}

template <class T>
void play_cast_successful_sound(
	const T& spell_data,
	const logic_step step, 
	const components::transform caster_transform,
	const entity_id subject
) {
	sound_existence_input sound;
	sound.delete_entity_after_effect_lifetime = true;
	sound.direct_listener = subject;
	sound.effect = spell_data.common.cast_successful_sound;

	sound.create_sound_effect_entity(
		step, 
		caster_transform,
		entity_id()
	).add_standard_components(step);
}

template <class T>
void play_cast_charging_sound(
	const T& spell_data,
	const logic_step step, 
	const components::transform caster_transform,
	const entity_id subject
) {
	sound_existence_input sound;
	sound.delete_entity_after_effect_lifetime = true;
	sound.direct_listener = subject;
	sound.effect = spell_data.charging_sound;

	sound.create_sound_effect_entity(
		step, 
		caster_transform,
		entity_id()
	).add_standard_components(step);
}