#pragma once
#include "augs/math/transform.h"
#include "augs/math/rects.h"
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
	/* For destructible sprites: the portion of the texture to render (0-1 space) */
	xywh texture_rect = xywh(0, 0, 1.0f, 1.0f);
};