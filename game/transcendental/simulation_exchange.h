#pragma once
#include <thread>
#include "augs/misc/streams.h"
#include "cosmic_entropy.h"
#include "cosmos.h"

struct input_context;

namespace augs {
	class fixed_delta;
}

class simulation_broadcast {
	augs::stream delta;
	std::thread delta_production;
public:
	unsigned delta_heartbeat_interval_in_steps = 10;

	void set_delta_heartbeat_interval(const augs::fixed_delta&, float ms);

	void start_producing_delta(const cosmos& base, const cosmos& enco);

	void simulate(const input_context&);
};

class simulation_receiver {
	std::vector<cosmic_entropy> jitter_buffer;
	cosmos last_snapshot;
	bool new_state_to_apply = false;

public:
	unsigned jitter_buffer_length = 3;

	void acquire_new_entropy(const cosmic_entropy&);
	void acquire_new_heartbeat(augs::stream& delta);

	void pre_solve(cosmos& into);

	void simulate(const input_context&);
};