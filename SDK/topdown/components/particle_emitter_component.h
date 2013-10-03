#pragma once
#include "utility/timer.h"
#include "entity_system/entity_ptr.h"

#include "../resources/particle_emitter_info.h"

namespace components {
	struct particle_emitter : public augmentations::entity_system::component {
		resources::particle_emitter_info* available_particle_effects;

		particle_emitter(resources::particle_emitter_info* available_particle_effects = nullptr)
			: available_particle_effects(available_particle_effects) {}
	};
}
