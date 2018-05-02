#pragma once
#include "game/components/transform_component.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"

template <class T>
void ignite_cast_sparkles(
	const T& spell_data,
	const logic_step step, 
	const entity_id subject
) {
	const auto& effect = spell_data.common.cast_sparkles;

	effect.start(
		step,
		{ particle_effect_start_input::at_entity(subject) }
	);
}

template <class T>
void ignite_charging_particles(
	const T& spell_data,
	const logic_step step, 
	const entity_id subject,
	const rgba col
) {
	auto effect = spell_data.charging_particles;
	effect.modifier.colorize = col;

	effect.start(
		step,
		{ particle_effect_start_input::at_entity(subject).set_homing(subject) }
	);
}

template <class T>
void play_cast_successful_sound(
	const T& spell_data,
	const logic_step step, 
	const entity_id subject
) {
	spell_data.common.cast_successful_sound.start(
		step,
		sound_effect_start_input::at_listener(subject)
	);
}

template <class T>
void play_cast_charging_sound(
	const T& spell_data,
	const logic_step step, 
	const entity_id subject
) {
	spell_data.charging_sound.start(
		step,
		sound_effect_start_input::at_listener(subject)
	);
}