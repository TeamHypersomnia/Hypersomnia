#pragma once
#include "augs/misc/profiler_mixin.h"
#include "augs/texture_atlas/atlas_profiler.h"

struct session_profiler : public augs::profiler_mixin<session_profiler> {
	session_profiler();

	// GEN INTROSPECTOR struct session_profiler
	augs::time_measurements fps;
	augs::time_measurements local_entropy;

	augs::time_measurements reloading_image_caches = std::size_t(1);
	augs::time_measurements reloading_sounds = std::size_t(1);

	augs::time_measurements viewables_readback;

	augs::time_measurements atlas_upload_to_gpu = std::size_t(1);

	augs::time_measurements camera_visibility_query;
	augs::time_measurements pbo_allocation;
	augs::amount_measurements<std::size_t> num_visible_entities;
	// END GEN INTROSPECTOR
};

struct network_profiler : public augs::profiler_mixin<network_profiler> {
	network_profiler();

	// GEN INTROSPECTOR struct network_profiler
	augs::time_measurements unpack_remote_steps;
	augs::time_measurements sending_commands_and_predict;
	augs::time_measurements sending_packets;
	augs::time_measurements remote_entropy;
	// END GEN INTROSPECTOR
};