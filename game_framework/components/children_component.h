#pragma once
#include <unordered_map>

#include "entity_system/entity.h"

namespace components {
	/* synchronizes death of multiple entities */
	struct children  {
		
		std::unordered_map<sub_entity_name, augs::entity_id> sub_entities_by_name;
		std::unordered_map<associated_entity_name, augs::entity_id> associated_entities_by_name;

		std::vector<augs::entity_id> sub_entities;

		void add_sub_entity(augs::entity_id p) {
			sub_entities.push_back(p);
		}

		void map_sub_entity(augs::entity_id p, sub_entity_name name) {
			sub_entities_by_name[name] = p;
		}

		void associate_entity(augs::entity_id p, associated_entity_name name) {
			associated_entities_by_name[name] = p;
		}
	};
}