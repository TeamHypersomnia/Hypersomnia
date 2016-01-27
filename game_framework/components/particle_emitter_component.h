#pragma once
#include "misc/timer.h"
#include "entity_system/entity.h"

#include "../resources/particle_emitter_info.h"

namespace components {
	struct particle_emitter  {
		resources::particle_emitter_info* available_particle_effects;

		resources::particle_effect* get_effect(int eff) {
			return &available_particle_effects->at(eff);
		}

		particle_emitter(resources::particle_emitter_info* available_particle_effects = nullptr)
			: available_particle_effects(available_particle_effects) {}
	};
}
