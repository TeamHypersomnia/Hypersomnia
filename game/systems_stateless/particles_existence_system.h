#pragma once

#include "game/messages/create_particle_effect.h"
#include "game/transcendental/step_declaration.h"

class particles_existence_system {
public:
	void displace_streams_and_destroy_dead_streams(const logic_step) const;

	void game_responses_to_particle_effects(const logic_step) const;
	void create_particle_effects(const logic_step) const;
	
	entity_handle create_particle_effect_entity(
		cosmos&, 
		const messages::create_particle_effect
	) const;
	
	void create_particle_effect_components(
		components::transform& out_transform,
		components::particles_existence& out_existence,
		components::position_copying& out_copying,
		cosmos&, 
		const messages::create_particle_effect
	) const;
};