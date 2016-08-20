#include "simulation_exchange.h"
#include "cosmic_delta.h"

#include "game/transcendental/types_specification/all_component_includes.h"

void simulation_broadcast::set_delta_heartbeat_interval(const augs::fixed_delta& dt, float ms) {
	delta_heartbeat_interval_in_steps = static_cast<unsigned>(ms / dt.in_milliseconds());
}

void simulation_receiver::acquire_new_entropy(const cosmic_entropy& entropy) {
	jitter_buffer.push_back(entropy);
}

void simulation_receiver::acquire_new_heartbeat(augs::stream& delta) {
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

void simulation_broadcast::simulate(const input_context& context) {
}

void simulation_receiver::simulate(const input_context& context) {
}