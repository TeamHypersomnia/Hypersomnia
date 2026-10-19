#pragma once
#include "augs/pad_bytes.h"
#include "augs/math/declare_math.h"

/*
	A portal's pixel particles - for making new portal effects, in developer builds (Settings > Portal effects: PARTICLES);
	the game draws the portals with a shader approximating them (portal_swirl.fsh).

	"count" of them are alive at once, each living lifetime_secs.
	With scale_count_with_size, portals smaller than 768x768 get proportionally fewer (by area).
	They spawn uniformly over the portal's disc, in its inner or outer ring color (randomly),
	turning into the outer ring color from end_color_from of their lifetime - so any color preset applies.

	Each moves away from the center at a constant random speed between radial_speed_min and radial_speed_max
	(in the portal's radii per second, negative - towards the center) and around it at spin degrees per second.
	At edge_radius_mult times the portal's radius they stop and slide along the edge, marking it with a subtle circle -
	each up to edge_softness of that radius before it, so that the circle is a soft band, not a saturated line -
	except edge_leak of them, going past it as decoration - from then on fading out over leak_fade_secs,
	keeping their size, however long they had left to live.

	Over the first grow_in and the last shrink_out of their lifetime they scale from and to nothing,
	over fade_in and fade_out - their alpha, up to max_opacity.

	A portal coming into view gets its particles spawned as if they had been there all along.
*/

struct continuous_rings_particles {
	// GEN INTROSPECTOR struct continuous_rings_particles
	int count = 5000;
	real32 lifetime_secs = 8.f;
	real32 size_mult = 1.f;
	real32 max_opacity = 0.07f;
	real32 fade_in = 0.2f;
	real32 fade_out = 0.3f;
	real32 grow_in = 0.3f;
	real32 shrink_out = 0.3f;
	real32 radial_speed_min = -0.03f;
	real32 radial_speed_max = 0.3f;
	real32 spin = 8.f;
	real32 end_color_from = 0.1f;
	real32 edge_radius_mult = 1.f;
	real32 edge_softness = 0.1f;
	real32 edge_leak = 0.25f;
	real32 leak_fade_secs = 4.f;
	bool scale_count_with_size = true;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR

	bool operator==(const continuous_rings_particles& b) const = default;
};
