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
	struct packaged_step {
		enum class type {
			INVALID,
			NEW_ENTROPY,
			NEW_ENTROPY_WITH_HEARTBEAT
		} step_type = type::INVALID;

		bool shall_resubstantiate = false;
		augs::stream delta;
		guid_mapped_entropy entropy;
	};

	static packaged_step read_entropy(augs::stream&);
	static void write_entropy(augs::stream&, const packaged_step&);

public:
	static void write_packaged_step_to_stream(augs::stream& output, const packaged_step&);

	static packaged_step read_entropy_for_next_step(augs::stream&);
	static packaged_step read_entropy_with_heartbeat_for_next_step(augs::stream&);
};
