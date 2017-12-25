#pragma once
#include <unordered_map>
#include "augs/pad_bytes.h"
#include "game/components/type_component_declaration.h"

using entity_description_type = entity_name_type;

struct entity_type {
	// GEN INTROSPECTOR class entity_type
	entity_name_type name;
	entity_description_type description;
	// END GEN INTROSPECTOR

	bool operator==(const entity_type& b) const {
		return 
			name == b.name 
			&& description == b.description
		;
	}
};

struct entity_types {
	// GEN INTROSPECTOR class entity_types
	entity_type_id next_type_id = 1;
	std::unordered_map<entity_type_id, entity_type> metas;
	// END GEN INTROSPECTOR
};