#pragma once
#include "simulation_exchange.h"

class simulation_receiver : public simulation_exchange {
public:
	augs::jitter_buffer<command> jitter_buffer;

	class unpacked_steps {
		friend class simulation_receiver;
		std::vector<cosmic_entropy> steps_for_proper_cosmos;
	public:
		bool use_extrapolated_cosmos = true;

		bool has_next_entropy() const;
		cosmic_entropy unpack_next_entropy(const cosmos& guid_mapper);
	};

	void read_entropy_for_next_step(augs::stream&);
	void read_entropy_with_heartbeat_for_next_step(augs::stream&);

	unpacked_steps unpack_deterministic_steps(cosmos& proper_cosmos, cosmos& extrapolated_cosmos, cosmos& last_delta_unpacked);
};