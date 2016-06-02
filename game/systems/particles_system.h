#pragma once
#include <random>
#include "misc/timer.h"

#include "entity_system/processing_system.h"

#include "../components/particle_effect_response_component.h"
#include "../components/particle_group_component.h"

using namespace augs;

class particles_system : public processing_system_templated<components::particle_group> {
public:
	using processing_system_templated::processing_system_templated;

	static entity_id create_refreshable_particle_group(world&);
	void spawn_particle(components::particle_group::stream&, const vec2&, float, float spread, const resources::emission&);
	void step_streams_and_particles();

	void destroy_dead_streams();

	void game_responses_to_particle_effects();
	void create_particle_effects();
};