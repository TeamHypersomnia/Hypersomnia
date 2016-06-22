#pragma once
#include "game/components/particle_effect_response_component.h"
#include "game/components/particle_group_component.h"

class cosmos;

class particles_system {
public:
	static entity_id create_refreshable_particle_group(cosmos&);
	void spawn_particle(components::particle_group::stream&, const vec2&, float, float spread, const resources::emission&);
	void step_streams_and_particles();

	void destroy_dead_streams();

	void game_responses_to_particle_effects();
	void create_particle_effects();
};