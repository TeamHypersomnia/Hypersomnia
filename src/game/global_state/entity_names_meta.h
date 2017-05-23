#pragma once
#include <unordered_map>
#include "augs/padding_byte.h"
#include "game/components/name_component_declaration.h"

using entity_details_type = entity_name_type;

struct entity_name_meta {
	// GEN INTROSPECTOR struct entity_name_meta
	bool stackable = false;
	std::array<padding_byte, 3> pad;
	entity_details_type details = L"No description";
	// END GEN INTROSPECTOR
};

using entity_names_meta = std::unordered_map<components::name, entity_name_meta>;