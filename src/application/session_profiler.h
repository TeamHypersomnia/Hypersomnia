#pragma once
#include "augs/misc/profiler_mixin.h"

class session_profiler : public augs::profiler_mixin<session_profiler> {
public:
	session_profiler();

	// GEN INTROSPECTOR class session_profiler
	augs::time_measurements fps;
	augs::time_measurements frame;
	augs::time_measurements local_entropy;
	augs::amount_measurements<std::size_t> triangles;
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