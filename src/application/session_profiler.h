#pragma once
#include "augs/misc/profiler_mixin.h"
#include "augs/texture_atlas/texture_atlas_profiler.h"

class session_profiler : public augs::profiler_mixin<session_profiler> {
public:
	session_profiler();

	// GEN INTROSPECTOR class session_profiler
	augs::time_measurements fps;
	augs::time_measurements local_entropy;

	augs::time_measurements reloading_images;
	augs::time_measurements reloading_sounds;

	augs::time_measurements viewables_readback;

	atlas_profiler atlas;
	augs::time_measurements atlas_upload_to_gpu = std::size_t(1);

	augs::time_measurements determining_viewables_to_preload;
	augs::time_measurements camera_visibility_query;
	augs::amount_measurements<std::size_t> num_visible_entities;
	// END GEN INTROSPECTOR
};

class network_profiler : public augs::profiler_mixin<network_profiler> {
public:
	network_profiler();

	// GEN INTROSPECTOR class network_profiler
	augs::time_measurements unpack_remote_steps;
	augs::time_measurements sending_commands_and_predict;
	augs::time_measurements sending_packets;
	augs::time_measurements remote_entropy;
	// END GEN INTROSPECTOR
};