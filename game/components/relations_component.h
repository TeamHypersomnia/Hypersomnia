#pragma once
#include <unordered_map>
#include <vector>
#include "game/entity_id.h"

namespace components {
	struct relations {
		std::unordered_map<sub_aggregate_name, entity_id> sub_aggregates_by_name;
		std::unordered_map<sub_entity_name, entity_id> sub_entities_by_name;
		std::unordered_map<associated_entity_name, entity_id> associated_entities_by_name;

		std::vector<entity_id> sub_entities;
		entity_id parent;
		entity_id self_id;
		sub_entity_name name_as_sub_entity = sub_entity_name::INVALID;
		sub_aggregate_name name_as_sub_aggregate = sub_aggregate_name::INVALID;
	};
}
