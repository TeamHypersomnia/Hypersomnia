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

class simulation_exchange {
public:
	struct command {
		enum class type {
			INVALID,
			NEW_ENTROPY,
			NEW_ENTROPY_WITH_HEARTBEAT
		} command_type = type::INVALID;

		augs::stream delta;
		cosmic_entropy entropy;
	};

	static command read_entropy(augs::stream&);

protected:
	static std::vector<command> read_commands_from_stream(augs::stream&);
	static void write_commands_to_stream(const cosmic_entropy&, const cosmos& guid_mapper);

	static command read_entropy_for_next_step(augs::stream&);
	static command read_entropy_with_heartbeat_for_next_step(augs::stream&);
};

class simulation_broadcast : public simulation_exchange {
	augs::stream delta;
	std::thread delta_production;
	unsigned steps_since_last_heartbeat = 0;
	
	cosmos::significant_state last_state_snapshot;

public:
	unsigned delta_heartbeat_interval_in_steps = 10;

	void set_delta_heartbeat_interval(const augs::fixed_delta&, float ms);

	void start_producing_delta(const cosmos& base, const cosmos& enco);
};

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