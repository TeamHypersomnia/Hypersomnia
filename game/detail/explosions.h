#pragma once
#include "game/assets/sound_buffer_id.h"
#include "game/components/transform_component.h"
#include "game/transcendental/logic_step.h"
#include "game/enums/explosion_type.h"

struct standard_explosion_input {
	const logic_step step;
	
	standard_explosion_input(const logic_step step)
		: step(step)
	{
	}
	
	components::transform explosion_location;
	entity_id subject_if_any = entity_id();
	float effective_radius = 250.f;
	float damage = 88.f;
	float impact_force = 150.f;
	rgba inner_ring_color = cyan;
	rgba outer_ring_color = white;
	assets::sound_buffer_id sound_effect = assets::sound_buffer_id::EXPLOSION;
	float sound_gain = 1.f;
	explosion_type type = explosion_type::FORCE;
};

void standard_explosion(const standard_explosion_input);