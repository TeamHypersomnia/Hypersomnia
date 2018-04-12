#pragma once
#include "augs/misc/profiler_mixin.h"

class audiovisual_profiler : public augs::profiler_mixin<audiovisual_profiler> {
public:
	audiovisual_profiler();

	// GEN INTROSPECTOR class audiovisual_profiler
	augs::time_measurements advance;
	augs::time_measurements interpolation;
	augs::time_measurements integrate_particles;
	augs::time_measurements advance_particle_streams;
	augs::time_measurements wandering_pixels;
	augs::time_measurements sound_logic;

	augs::time_measurements post_solve;
	augs::time_measurements post_cleanup;
	// END GEN INTROSPECTOR
};