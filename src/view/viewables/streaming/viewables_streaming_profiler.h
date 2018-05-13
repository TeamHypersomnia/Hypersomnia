#pragma once
#include "augs/misc/measurements.h"
#include "augs/misc/profiler_mixin.h"

struct viewables_streaming_profiler : public augs::profiler_mixin<viewables_streaming_profiler> {
	viewables_streaming_profiler();

	// GEN INTROSPECTOR struct viewables_streaming_profiler
	augs::time_measurements reloading_sounds = std::size_t(1);
	augs::time_measurements viewables_readback = std::size_t(1);
	augs::time_measurements atlas_upload_to_gpu = std::size_t(1);
	augs::time_measurements neon_regeneration = std::size_t(1);
	augs::time_measurements pbo_allocation = std::size_t(1);
	// END GEN INTROSPECTOR
};
