#pragma once
#include <unordered_map>
#include "augs/padding_byte.h"
#include "game/components/name_component_declaration.h"
#include "augs/templates/container_templates.h"

using entity_description_type = entity_name_type;

namespace augs {
	struct introspection_access;
}

class entity_name_meta {
	// GEN INTROSPECTOR class entity_name_meta
	friend struct augs::introspection_access;
	friend class entity_name_metas;
	
	entity_name_type name;
public:
	bool stackable = false;
	entity_description_type description = L"No description";
	// END GEN INTROSPECTOR

	const auto& get_name() const {
		return name;
	}

	entity_name_meta(const entity_name_type& name = L"Invalid") : name(name) {}
};

class entity_name_metas {
	// GEN INTROSPECTOR class entity_name_metas
	friend struct augs::introspection_access;

	entity_name_id next_name_id = 0;

	std::unordered_map<entity_name_type, entity_name_id> name_to_id;
	std::unordered_map<entity_name_id, entity_name_meta> metas;
	// END GEN INTROSPECTOR
public:
	entity_name_metas() {
		make_id_for(L"Unnamed");
	}

	entity_name_meta& get_meta(const entity_name_id id) {
		return metas.at(id);
	}

	const entity_name_meta& get_meta(const entity_name_id id) const {
		return metas.at(id);
	}

	entity_name_id get_id_for(const entity_name_type& name) const {
		return found_or_default(name_to_id, name, std::numeric_limits<entity_name_id>::max());
	}

	entity_name_id make_id_for(const entity_name_type& name) {
		const auto it = name_to_id.find(name);

		if (it == name_to_id.end()) {
			const auto inserted = name_to_id.insert(it, { name, next_name_id });
			metas.emplace(next_name_id, name);
			return next_name_id++;
		}
		else {
			return (*it).second;
		}
	}
};