#pragma once
#include <vector>
#include "entity_system/component.h"
#include "entity_system/entity.h"

namespace components {
	/* synchronizes death of multiple entities */
	struct children : public augs::entity_system::component {
		std::vector<augs::entity_system::entity_id> children_entities;

		void add(const augs::entity_system::entity_id& p) {
			children_entities.push_back(p);
		}
	};
}