#pragma once
#include "game/assets/sound_buffer_id.h"
#include "game/components/transform_component.h"
#include "game/transcendental/step_declaration.h"

class cosmos;
struct rgba;
struct entity_id;

void standard_explosion(
	const logic_step step,
	components::transform location,
	const entity_id subject_if_any,
	const float effective_radius,
	const rgba inner_ring_color,
	const rgba outer_ring_color,
	const assets::sound_buffer_id sound_effect, 
	const float sound_gain = 1.f
);