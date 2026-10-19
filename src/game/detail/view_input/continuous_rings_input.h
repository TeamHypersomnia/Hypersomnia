#pragma once
#include "augs/pad_bytes.h"
#include "augs/graphics/rgba.h"
#include "game/detail/view_input/portal_particles_def.h"

/*
	Note the radius is implied by other properties like object size.

	pixel_particles replaces the flat rings with the explosions' pixel art particles (see continuous_rings_particles) -
	or, as the client's portal_effects setting picks, with a shader approximating them (portal_swirl.fsh).
*/

struct continuous_rings_input {
	// GEN INTROSPECTOR struct continuous_rings_input
	float effect_speed = 1.0f;

	rgba inner_color = cyan;
	rgba outer_color = turquoise;

	continuous_rings_particles particles;
	bool pixel_particles = true;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR

	bool operator==(const continuous_rings_input& b) const = default;
};
