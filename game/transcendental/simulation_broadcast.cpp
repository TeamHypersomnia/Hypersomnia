#include "simulation_broadcast.h"

#include "game/transcendental/types_specification/all_component_includes.h"

void simulation_broadcast::push_duplicate(const cosmos& from) {
	last_state_snapshot = from.significant;
}

void simulation_broadcast::set_state_heartbeat_interval(const augs::fixed_delta& dt, float ms) {
	state_heartbeat_interval_in_steps = static_cast<unsigned>(ms / dt.in_milliseconds());
}

//void simulation_broadcast::read_client_commands(augs::stream& in) {
//
//}

cosmic_entropy unpack_commands_from_all_endpoints(const cosmos& guid_mapper) {

}
