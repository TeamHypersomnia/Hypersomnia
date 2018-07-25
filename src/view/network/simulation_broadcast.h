#pragma once
#include <thread>
#include "game/cosmos/cosmos.h"
#include "augs/network/reliable_channel.h"

class entropy_accepter {
	augs::network::reliable_channel channel;
public:

	void read_entropy_for_next_step(augs::memory_stream&);

	cosmic_entropy unpack_entropy_for_next_step(const cosmos& guid_mapper);
};

class simulation_broadcast {
	augs::memory_stream delta;
	std::thread delta_production;
	unsigned steps_since_last_heartbeat = 0;

	cosmos_solvable_significant last_state_snapshot;

	void push_duplicate(const cosmos& from);

	void start_producing_delta(const cosmos& base);

public:
	unsigned state_heartbeat_interval_in_steps = 10;

	void set_state_heartbeat_interval(const augs::delta&, float ms);

};