#include "augs/templates.h"
#include "simulation_receiver.h"
#include "game/transcendental/cosmic_delta.h"
#include "augs/network/network_client.h"

#include "game/transcendental/network_commands.h"
#include "game/transcendental/cosmos.h"

void simulation_receiver::acquire_next_packaged_step(const step_packaged_for_network& step) {
	jitter_buffer.acquire_new_command(step);
}