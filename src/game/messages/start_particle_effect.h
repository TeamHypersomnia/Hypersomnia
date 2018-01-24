#pragma once
#include "message.h"
#include "augs/math/vec2.h"

#include "game/transcendental/entity_id.h"
#include "game/components/transform_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/assets/ids/particle_effect_id.h"

namespace messages {
	struct start_particle_effect : message {
		particle_effect_input effect;
		particle_effect_start_input start;
	};
}
