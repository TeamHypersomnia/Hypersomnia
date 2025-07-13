#pragma once
#include <cstddef>
#include "augs/misc/measurements.h"
#include "augs/misc/profiler_mixin.h"
#include "augs/math/vec2.h"

struct atlas_profiler : public augs::profiler_mixin<atlas_profiler> {
	atlas_profiler();

	// GEN INTROSPECTOR struct atlas_profiler
	augs::time_measurements whole_regeneration = std::size_t(1);
	augs::time_measurements gathering_subjects = std::size_t(1);
	augs::time_measurements unpacking_results = std::size_t(1);

	augs::time_measurements loading_image_sizes = std::size_t(1);
	augs::time_measurements loading_images = std::size_t(1);
	augs::time_measurements making_worker_inputs = std::size_t(1);
	augs::time_measurements decoding_images = std::size_t(1);

	augs::time_measurements loading_fonts = std::size_t(1);

	augs::time_measurements blitting_images = std::size_t(1);
	augs::time_measurements blitting_fonts = std::size_t(1);

	augs::amount_measurements<vec2u> atlas_size = std::size_t(1);
	augs::amount_measurements<unsigned> atlas_height = std::size_t(1);
	augs::time_measurements resizing_image = std::size_t(1);

	augs::time_measurements packing = std::size_t(1);
	augs::amount_measurements<std::size_t> subjects_count = std::size_t(1);
	augs::amount_measurements<std::size_t> wasted_space = std::size_t(1);
	augs::amount_measurements<double> wasted_space_percent = std::size_t(1);
	// END GEN INTROSPECTOR
};

