#pragma once
#include "particle_emitter_component.h"

namespace components {
	struct particle_stream : public augmentations::entity_system::component {
		components::particle_emitter::emission* info;

		float lifetime_ms;
		float max_lifetime_ms;
		float particles_to_spawn;

		particle_stream(components::particle_emitter::emission* info) : info(info), lifetime_ms(0.f), particles_to_spawn(0.f) {}
	};
}