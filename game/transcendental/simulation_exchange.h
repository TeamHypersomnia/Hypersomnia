#pragma once
#include <thread>
#include "cosmos.h"

namespace augs {
	class fixed_delta;
}

class simulation_broadcast {
	augs::bit_stream delta;
	std::thread delta_production;
public:
	unsigned delta_heartbeat_interval_in_steps = 10;

	void set_delta_heartbeat_interval(const augs::fixed_delta&, float ms);

	void start_producing_delta(const cosmos& base, const cosmos& enco);
};

class simulation_receiver {
	std::vector<cosmic_entropy> jitter_buffer;
	cosmos last_snapshot;

public:
	unsigned jitter_buffer_length = 3;

};