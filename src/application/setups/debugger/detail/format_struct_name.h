#pragma once
#include "augs/string/string_templates.h"
#include "game/organization/all_entity_types.h"

template <class T>
auto format_struct_name(const T&) {
	auto result = format_field_name(get_type_name_strip_namespace<T>());
	result[0] = std::toupper(result[0]);

	/* These look ugly with automated names */

	if constexpr(std::is_same_v<T, invariants::sprite>) {
		result = "Sprite";
	}	

	if constexpr(std::is_same_v<T, invariants::polygon>) {
		result = "Polygon";
	}

	if constexpr(std::is_same_v<T, components::transform>) {
		result = "Transform";
	}	

	if constexpr(std::is_same_v<T, components::position>) {
		result = "Position";
	}	

	return result;
}

