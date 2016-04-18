#pragma once
#include "misc/timer.h"
#include "entity_system/entity.h"

#include "../assets/particle_effect_response.h"

namespace components {
	struct particle_effect_response {
		assets::particle_effect_response_id available_particle_effects;
	};
}
