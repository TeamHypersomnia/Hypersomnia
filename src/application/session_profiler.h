#pragma once
#include "augs/misc/profiler_mixin.h"
#include "augs/texture_atlas/atlas_profiler.h"

struct session_profiler : public augs::profiler_mixin<session_profiler> {
	session_profiler();

	// GEN INTROSPECTOR struct session_profiler
	augs::time_measurements fps;
	augs::time_measurements swap_buffers;
	augs::time_measurements local_entropy;
	// END GEN INTROSPECTOR
};

struct network_profiler : public augs::profiler_mixin<network_profiler> {
	network_profiler();

	// GEN INTROSPECTOR struct network_profiler
	augs::amount_measurements<std::size_t> predicted_steps = 1;
	augs::amount_measurements<std::size_t> accepted_commands = 1;

	augs::time_measurements unpacking_remote_steps;
	augs::time_measurements sending_messages;
	augs::time_measurements receiving_messages;
	// END GEN INTROSPECTOR
};