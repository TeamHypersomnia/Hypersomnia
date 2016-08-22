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
	static void write_entropy(augs::stream&, const command&, const cosmos& guid_mapper);

protected:
	static void write_command_to_stream(augs::stream& output, const command&, const cosmos& guid_mapper);

	static command read_entropy_for_next_step(augs::stream&);
	static command read_entropy_with_heartbeat_for_next_step(augs::stream&);
};
