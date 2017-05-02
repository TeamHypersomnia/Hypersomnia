#pragma once
#include "game/messages/create_particle_effect.h"
#include "game/transcendental/step_declaration.h"

class particles_existence_system {
public:
	void displace_streams_and_destroy_dead_streams(const logic_step) const;

	void game_responses_to_particle_effects(const logic_step) const;
};