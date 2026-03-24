#pragma once
#include "game/cosmos/entity_id.h"
#include "game/messages/visibility_information.h"

struct exploding_ring_input {
	float outer_radius_start_value = 0.f;
	float outer_radius_end_value = 0.f;
	float inner_radius_start_value = 0.f;
	float inner_radius_end_value = 0.f;

	float maximum_duration_seconds = 0.f;

	bool emit_particles_on_ring = false;
	bool emit_light = true;

	float final_alpha = 0.0f;
	float halve_per_ms = -1.0f;
	float fixed_thickness = -1.0f;

	vec2 center;
	entity_id target;

	messages::visibility_information_response visibility;
	rgba color = white;
};