#pragma once
#include "misc/timer.h"
#include "game/entity_id.h"

#include "game/assets/particle_effect_response_id.h"
#include "game/resources/particle_effect.h"

namespace components {
	struct particle_effect_response {
		assets::particle_effect_response_id response;
		resources::particle_effect_modifier modifier;
	};
}
