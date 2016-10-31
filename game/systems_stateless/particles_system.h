#pragma once
#include "game/components/particle_effect_response_component.h"
#include "game/components/particle_group_component.h"

class logic_step;
struct randomization;

class particles_system {
public:
	entity_id create_refreshable_particle_group(logic_step&) const;
	void spawn_particle(randomization&, components::particle_group::stream&, const vec2&, float, float spread, const resources::emission&) const;
	void step_streams_and_particles(logic_step&) const;

	void destroy_dead_streams(logic_step&) const;

	void game_responses_to_particle_effects(logic_step&) const;
	void create_particle_effects(logic_step&) const;
};