#pragma once
#include <unordered_map>
#include "augs/pad_bytes.h"
#include "game/components/name_component_declaration.h"

using entity_description_type = entity_name_type;

namespace augs {
	struct introspection_access;
}

class entity_name_meta {
	// GEN INTROSPECTOR class entity_name_meta
	friend augs::introspection_access;
	friend class name_system;
	
	entity_name_type name;
public:
	bool stackable = false;
	entity_description_type description = L"No description";
	// END GEN INTROSPECTOR

	bool operator==(const entity_name_meta& b) const {
		return name == b.name && stackable == b.stackable && description == b.description;
	}

	const auto& get_name() const {
		return name;
	}

	bool has_description() const {
		return description != L"No description";
	}

	entity_name_meta(const entity_name_type& name = L"Invalid") : name(name) {}
};

class entity_name_metas {
	friend augs::introspection_access;
	friend class name_system;

	// GEN INTROSPECTOR class entity_name_metas
	entity_name_id next_name_id = 1;
	std::unordered_map<entity_name_id, entity_name_meta> metas;
	// END GEN INTROSPECTOR
public:
	entity_name_metas() {
		clear();
	}

	void clear() {
		metas.clear();
		metas[0] = entity_name_meta(L"Unnamed");
	}

	entity_name_meta& get_meta(const entity_name_id id) {
		return metas.at(id);
	}

	const entity_name_meta& get_meta(const entity_name_id id) const {
		return metas.at(id);
	}
};