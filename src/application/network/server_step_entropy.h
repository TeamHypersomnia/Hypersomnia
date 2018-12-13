#pragma once
#include "game/modes/mode_entropy.h"

using server_step_entropy = mode_entropy;

struct server_step_entropy_for_client {
	// GEN INTROSPECTOR struct server_step_entropy_for_client
	server_step_entropy total;
	uint8_t num_entropies_accepted = 0;
	// END GEN INTROSPECTOR

	/* TODO: num_ready_upcoming_entropies */
};

