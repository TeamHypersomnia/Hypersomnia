#pragma once
#include "augs/misc/profiler_mixin.h"

struct server_profiler : public augs::profiler_mixin<server_profiler> {
	server_profiler();

	// GEN INTROSPECTOR struct server_profiler
	augs::time_measurements step;
	augs::time_measurements advance_adapter;
	augs::time_measurements advance_clients_state;
	augs::time_measurements solve_simulation;
	augs::time_measurements send_entropies;
	augs::time_measurements send_packets;
	// END GEN INTROSPECTOR
};

