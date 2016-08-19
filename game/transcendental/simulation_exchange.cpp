#include "simulation_exchange.h"
#include "cosmic_delta.h"

void simulation_receiver::acquire_new_entropy(const cosmic_entropy& entropy) {
	jitter_buffer.push_back(entropy);
}

void simulation_receiver::acquire_new_heartbeat(augs::bit_stream& delta) {
	cosmic_delta::decode(last_snapshot, delta);

	new_state_to_apply = true;
}

void simulation_receiver::pre_solve(cosmos& into) {
	if (new_state_to_apply) {
		into = last_snapshot;
		new_state_to_apply = false;
	}

	if (jitter_buffer.size() > 0) {
		auto next_entropy = jitter_buffer.front();
		jitter_buffer.erase(jitter_buffer.begin());
	}
}
