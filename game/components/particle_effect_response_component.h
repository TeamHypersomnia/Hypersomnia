#pragma once
#include "misc/timer.h"
#include "entity_system/entity.h"

#include "../assets/particle_effect_response_id.h"
#include "../resources/particle_effect.h"

namespace components {
	struct particle_effect_response {
		assets::particle_effect_response_id response;
		resources::particle_effect_modifier modifier;
	};
}
