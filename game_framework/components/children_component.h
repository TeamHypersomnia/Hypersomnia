#pragma once
#include <unordered_map>
#include "entity_system/component.h"
#include "entity_system/entity.h"

namespace components {
	/* synchronizes death of multiple entities */
	struct children : public augs::component {
		enum sub_entity_name {
			CHARACTER_CROSSHAIR,
			CHARACTER_LEGS
		};
		
		std::unordered_map<sub_entity_name, augs::entity_id> sub_entities_by_name;
		std::vector<augs::entity_id> sub_entities;

		void add_sub_entity(augs::entity_id p) {
			sub_entities.push_back(p);
		}

		void map_sub_entity(augs::entity_id p, sub_entity_name name) {
			sub_entities_by_name[name] = p;
		}
	};
}