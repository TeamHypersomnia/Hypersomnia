#include "simulation_broadcast.h"

#include "game/organization/all_component_includes.h"

void simulation_broadcast::push_duplicate(const cosmos& from) {
	last_state_snapshot = from.get_solvable().significant;
}

void simulation_broadcast::set_state_heartbeat_interval(const augs::delta& dt, float ms) {
	state_heartbeat_interval_in_steps = static_cast<unsigned>(ms / dt.in_milliseconds());
}

//void simulation_broadcast::read_client_commands(augs::memory_stream& in) {
//
//}

cosmic_entropy unpack_commands_from_all_endpoints(const cosmos& /* guid_mapper */) {
	return cosmic_entropy();
}
