#pragma once
#include "augs/misc/profiler_mixin.h"

struct frame_profiler : public augs::profiler_mixin<frame_profiler> {
	frame_profiler();

	// GEN INTROSPECTOR struct frame_profiler
	augs::time_measurements total;
	augs::amount_measurements<std::size_t> num_triangles = 1;
	augs::amount_measurements<std::size_t> light_raycasts = 1;

	augs::time_measurements rendering_script;
	augs::time_measurements light_visibility;
	augs::time_measurements light_rendering;
	augs::amount_measurements<std::size_t> num_visible_lights = 1;

	augs::time_measurements particles_rendering;
	// END GEN INTROSPECTOR
};

