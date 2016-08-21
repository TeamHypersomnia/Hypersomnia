#pragma once
#include <thread>
#include "augs/misc/streams.h"
#include "augs/misc/jitter_buffer.h"

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

public:
	struct command {
		enum class type {
			INVALID,
			NEW_INPUT,
			NEW_HEARTBEAT
		} command_type = type::INVALID;

		augs::stream delta;
		cosmic_entropy entropy;
	};

	augs::jitter_buffer<command> jitter_buffer;

	void acquire_new_entropy(const cosmic_entropy&);
	void acquire_new_entropy_with_heartbeat(const cosmic_entropy&, const augs::stream&);

	struct unpacked_steps {
		bool use_extrapolated_cosmos = true;

		std::vector<cosmic_entropy> steps_for_proper_cosmos;
	};

	unpacked_steps unpack_deterministic_steps(cosmos& proper_cosmos, cosmos& extrapolated_cosmos, cosmos& last_delta_unpacked);
};