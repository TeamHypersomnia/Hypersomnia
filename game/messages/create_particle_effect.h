#pragma once
#include "message.h"
#include "augs/math/vec2.h"

#include "game/transcendental/entity_id.h"
#include "game/components/transform_component.h"
#include "game/components/particles_existence_component.h"
#include "game/assets/particle_effect_id.h"

namespace messages {
	struct create_particle_effect : public message {
		components::particles_existence::effect_input input;
		components::transform place_of_birth;
	};
}