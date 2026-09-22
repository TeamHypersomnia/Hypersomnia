#pragma once
#include "augs/math/transform.h"
#include "augs/drawing/drawing.h"

template <class T>
struct basic_drawing_input_base {
	const augs::drawer output;
	const camera_cone cone;
	
	basic_drawing_input_base(
		const augs::drawer output,
		const camera_cone cone
	) : 
		output(output),
		cone(cone)
	{}

	basic_transform<T> renderable_transform;
	rgba colorize = white;
	bool use_neon_map = false;
	bool disable_special_effects = false;
};

using drawing_input_base = basic_drawing_input_base<real32>;

struct polygon_drawing_input : drawing_input_base {
	using drawing_input_base::drawing_input_base;

	double global_time_seconds = 0.0;
};

struct sprite_drawing_input : drawing_input_base {
	using drawing_input_base::drawing_input_base;

	double global_time_seconds = 0.0;
	vec2i tile_size = vec2i(0, 0);
	flip_flags flip;

	/*
		How far behind its own back the neon tail may reach, in world units.
		Negative imposes no limit.
	*/
	real32 max_neon_tail_behind = -1.f;

	/* Scales whatever extension survives that limit. */
	real32 neon_tail_extension_mult = 1.f;
};
