#pragma once
#include "message.h"
#include "math/vec2.h"

#include "game/entity_id.h"
#include "game/components/transform_component.h"
#include "game/assets/particle_effect_id.h"

namespace messages {
	struct create_particle_effect : public message {
		assets::particle_effect_id effect;
		resources::particle_effect_modifier modifier;

		entity_id target_group_to_refresh;

		bool local_transform = false;
		components::transform transform;
	};
}