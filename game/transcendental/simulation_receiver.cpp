#include "simulation_receiver.h"
#include "game/transcendental/cosmic_delta.h"
#include "augs/network/network_client.h"

#include "game/transcendental/network_commands.h"
#include "game/transcendental/cosmos.h"

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
