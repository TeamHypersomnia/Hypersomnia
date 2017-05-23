#pragma once
#include <unordered_map>
#include "game/components/name_component_declaration.h"

using entity_description_type = entity_name_type;

struct entity_names_meta {
	// GEN INTROSPECTOR struct entity_names_meta
	std::unordered_map<entity_name_type, entity_description_type> descriptions;
	// END GEN INTROSPECTOR
};