#pragma once


#include "game/transcendental/step_declaration.h"

class particles_existence_system {
public:
	void destroy_dead_streams(logic_step&) const;

	void game_responses_to_particle_effects(logic_step&) const;
	void create_particle_effects(logic_step&) const;
};