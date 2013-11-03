#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"

namespace components {
	struct lookat : public augmentations::entity_system::component {
		enum look_type {
			POSITION,
			VELOCITY,
			ACCELEARATION
		};

		unsigned look_mode;

		augmentations::entity_system::entity_ptr target;

		lookat(augmentations::entity_system::entity* target = nullptr, unsigned look_mode = look_type::POSITION) : target(target), look_mode(look_mode) {}
	};
}