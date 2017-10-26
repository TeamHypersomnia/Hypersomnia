#pragma once
#include "game/organization/all_components_declaration.h"
#include "augs/templates/for_each_std_get.h"

template <class F>
void for_each_component_type(F&& callback) {
	for_each_through_std_get(
		component_list_t<std::tuple>(),
		std::forward<F>(callback)
	);
}