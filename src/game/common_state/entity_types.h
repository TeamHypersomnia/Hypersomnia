#pragma once
#include <tuple>

#include "augs/pad_bytes.h"
#include "game/components/type_component_declaration.h"
#include "game/organization/all_component_includes.h"

using entity_description_type = entity_name_type;
using entity_initial_components = component_list_t<std::tuple>;
using definition_tuple = definition_list_t<augs::trivially_copyable_tuple>;

struct entity_type {
	// GEN INTROSPECTOR struct entity_type
	entity_name_type name;
	entity_description_type description;

	entity_initial_components initial_components;
	std::array<bool, DEFINITIONS_COUNT> enabled_definitions = {};
	// END GEN INTROSPECTOR

	bool operator==(const entity_type& b) const {
		return 
			name == b.name 
			&& description == b.description
		;
	}
};

#if STATICALLY_ALLOCATE_ENTITY_TYPES_NUM
#include "augs/misc/constant_size_vector.h"
static_assert(std::is_integral_v<entity_type_id>);
using entity_types_container = augs::constant_size_vector<entity_type, STATICALLY_ALLOCATE_ENTITY_TYPES_NUM>;
#else
#include <unordered_map>
using entity_types_container = std::unordered_map<entity_type_id, entity_type>;
#endif

struct entity_types {
	// GEN INTROSPECTOR struct entity_types
	entity_types_container types;
	// END GEN INTROSPECTOR

#if STATICALLY_ALLOCATE_ENTITY_TYPES_NUM
	auto& get_type(const entity_type_id id) {
		return types[id];
	}

	const auto& get_type(const entity_type_id id) const {
		return types[id];
	}
#else
	auto& get_type(const entity_type_id id) {
		return types.at(id);
	}

	const auto& get_type(const entity_type_id id) const {
		return types.at(id);
	}
#endif
};