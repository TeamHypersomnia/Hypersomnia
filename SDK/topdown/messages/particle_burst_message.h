#pragma once
#include "message.h"
#include "math/vec2d.h"

#include "entity_system/entity_ptr.h"

namespace resources {
	struct emission;
	typedef std::vector<emission> particle_effect;
}

namespace components {
	struct particle_group;
}

namespace messages {
	struct particle_burst_message : public message {
		enum burst_type {
			BULLET_IMPACT,
			WEAPON_SHOT
		} type;

		resources::particle_effect* set_effect;
		augmentations::entity_system::entity_ptr target_group_to_refresh;

		augmentations::vec2<> pos;
		float rotation;

		particle_burst_message() : set_effect(nullptr), rotation(0.f), target_group_to_refresh(nullptr) {}
	};
}