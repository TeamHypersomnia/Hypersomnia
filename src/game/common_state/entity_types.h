#pragma once
#include <unordered_map>
#include "augs/pad_bytes.h"
#include "game/components/name_component_declaration.h"

using entity_description_type = entity_name_type;

namespace augs {
	struct introspection_access;
}

class entity_type {
	// GEN INTROSPECTOR class entity_type
	friend augs::introspection_access;
	friend class name_cache;
	
	entity_name_type name;
public:
	entity_description_type description;
	// END GEN INTROSPECTOR

	bool operator==(const entity_type& b) const {
		return 
			name == b.name 
			&& description == b.description
		;
	}

	const auto& get_name() const {
		return name;
	}

	entity_type(const entity_name_type& name = {}) : name(name) {}
};

class entity_types {
	friend augs::introspection_access;
	friend class name_cache;

	// GEN INTROSPECTOR class entity_types
	entity_type_id next_type_id = 1;
	std::unordered_map<entity_type_id, entity_type> metas;
	// END GEN INTROSPECTOR

public:
	void clear() {
		metas.clear();
	}

	entity_type& get_type(const entity_type_id id) {
		return metas.at(id);
	}

	const entity_type& get_type(const entity_type_id id) const {
		return metas.at(id);
	}
};