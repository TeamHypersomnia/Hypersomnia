#pragma once
#include "message.h"
#include "math/vec2.h"

#include "entity_system/entity.h"
#include "../components/transform_component.h"
#include "../assets/particle_effect_id.h"

namespace messages {
	struct create_particle_effect : public message {
		assets::particle_effect_id effect;
		resources::particle_effect_modifier modifier;

		augs::entity_id target_group_to_refresh;

		bool local_transform = false;
		components::transform transform;
	};
}