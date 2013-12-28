#pragma once
#include <random>
#include "utility/timer.h"

#include "entity_system/processing_system.h"

#include "../components/particle_emitter_component.h"
#include "../components/particle_group_component.h"

using namespace augmentations;
using namespace entity_system;

class particle_emitter_system : public processing_system_templated<components::particle_emitter> {
public:
	static void spawn_particle(components::particle_group::stream&, const vec2<>&, float, const resources::emission&);
	void process_events(world&) override;
};