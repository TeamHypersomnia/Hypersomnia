#include "simulation_receiver.h"
#include "game/transcendental/cosmic_delta.h"

void simulation_receiver::read_entropy_for_next_step(augs::stream& in, bool skip) {
	auto result = simulation_exchange::read_entropy_for_next_step(in);
	
	if(!skip)
		jitter_buffer.acquire_new_command(result);
}

void simulation_receiver::read_entropy_with_heartbeat_for_next_step(augs::stream& in, bool skip) {
	auto result = simulation_exchange::read_entropy_with_heartbeat_for_next_step(in);

	if (!skip)
		jitter_buffer.acquire_new_command(result);
}

bool simulation_receiver::unpacked_steps::has_next_entropy() const {
	return steps_for_proper_cosmos.size() > 0;
}

cosmic_entropy simulation_receiver::unpacked_steps::unpack_next_entropy(const cosmos& guid_mapper) {
	ensure(has_next_entropy());

	cosmic_entropy result(steps_for_proper_cosmos.front(), guid_mapper);
	steps_for_proper_cosmos.erase(steps_for_proper_cosmos.begin());

	return std::move(result);
}