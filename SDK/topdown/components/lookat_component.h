#pragma once
#include "entity_system/entity.h"

namespace components {
	struct lookat : public augmentations::entity_system::component {
		enum class chase_type {
			POSITION,
			VELOCITY
		} type;

		augmentations::entity_system::entity* target;

		lookat(augmentations::entity_system::entity* target, chase_type type = chase_type::POSITION) : target(target), type(type) {}
	};
}