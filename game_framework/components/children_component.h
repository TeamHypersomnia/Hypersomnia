#pragma once
#include <vector>
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"

namespace components {
	/* synchronizes death of multiple entities */
	struct children : public augmentations::entity_system::component {
		std::vector<augmentations::entity_system::entity_ptr> children_entities;

		void add(const augmentations::entity_system::entity_ptr& p) {
			children_entities.push_back(p);
		}
	};
}