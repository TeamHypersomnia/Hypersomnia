#pragma once
#include "game/cosmos/entity_id.h"
#include "game/messages/visibility_information.h"
#include "game/detail/view_input/explosion_particles_def.h"

struct exploding_ring_input {
	float outer_radius_start_value = 0.f;
	float outer_radius_end_value = 0.f;
	float inner_radius_start_value = 0.f;
	float inner_radius_end_value = 0.f;

	float maximum_duration_seconds = 0.f;

	/*
		emit_ring_end_particles spawns the sparkles and smokes along the ring right before it vanishes.

		emit_explosion_particles fills the whole ring at its start with pixel art particles
		moving with the ring's edges, so they approximate the ring itself (see explosion_particles_def).
		The particles start from explosion_particles_hot_color, slightly brightened,
		or explosion_particles_palette_offset entries into the palette.

		draw_color_rings draws the ring itself as a flat colored shape.
		is_explosion_thin_ring marks the thin ring accompanying an explosion's ring -
		its thickness comes from the client's explosions_settings,
		and it vanishes by getting thinner (down to sub-pixel widths) instead of by fading its alpha.
	*/
	bool emit_ring_end_particles = false;
	bool emit_explosion_particles = false;
	bool draw_color_rings = true;
	bool is_explosion_thin_ring = false;
	bool emit_light = true;

	explosion_particles_def explosion_particles;
	float explosion_particles_palette_offset = 0.f;
	rgba explosion_particles_hot_color = white;

	float final_alpha = 0.0f;
	float halve_per_ms = -1.0f;
	float fixed_thickness = -1.0f;

	vec2 center;
	entity_id target;

	messages::visibility_information_response visibility;
	rgba color = white;
};