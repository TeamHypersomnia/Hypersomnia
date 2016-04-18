#pragma once
#include "message.h"
#include "math/vec2.h"

#include "entity_system/entity.h"
#include "../components/transform_component.h"

namespace resources {
	struct emission;
	typedef std::vector<emission> particle_effect;
}

namespace components {
	struct particle_group;
}

namespace messages {
	struct create_particle_effect : public message {
		resources::particle_effect effect;

		augs::entity_id target_group_to_refresh;

		bool local_transform = false;
		components::transform transform;
	};
}