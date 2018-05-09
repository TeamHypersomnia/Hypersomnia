#pragma once
#include "game/messages/visibility_information.h"

struct exploding_ring_input {
	float outer_radius_start_value = 0.f;
	float outer_radius_end_value = 0.f;
	float inner_radius_start_value = 0.f;
	float inner_radius_end_value = 0.f;

	float maximum_duration_seconds = 0.f;

	bool emit_particles_on_ring = false;

	vec2 center;

	messages::visibility_information_response visibility;
	rgba color = white;
};