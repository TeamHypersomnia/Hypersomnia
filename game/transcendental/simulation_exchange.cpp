#include "augs/misc/templated_readwrite.h"

#include "simulation_exchange.h"
#include "cosmic_delta.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/network_commands.h"
#include "game/transcendental/cosmos.h"

//void simulation_receiver::read_commands_from_stream(augs::stream& in) {
//	auto new_commands = simulation_exchange::read_commands_from_stream(in);
//	jitter_buffer.acquire_new_commands(new_commands.begin(), new_commands.end());
//}