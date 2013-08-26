#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"

namespace components {
	struct lookat : public augmentations::entity_system::component {
		enum class chase_type {
			POSITION,
			VELOCITY
		} type;

		augmentations::entity_system::entity_ptr target;

		lookat(augmentations::entity_system::entity* target, chase_type type = chase_type::POSITION) : target(target), type(type) {}
	};
}