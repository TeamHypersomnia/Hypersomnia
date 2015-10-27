#pragma once
#include "message.h"
#include "math/vec2d.h"

#include "entity_system/entity.h"
#include "../resources/particle_emitter_info.h"

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
			WEAPON_SHOT,

			CUSTOM
		};

		int type = CUSTOM;

		resources::particle_effect effect;

		void set_effect(resources::particle_effect* eff) {
			effect = *eff;
		}

		augs::entity_system::entity_id target_group_to_refresh;

		bool local_transform = false;

		augs::vec2<> pos;
		float rotation = 0.f;
	};
}