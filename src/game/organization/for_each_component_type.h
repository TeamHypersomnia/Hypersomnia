#pragma once
#include "game/organization/all_components_declaration.h"
#include "augs/templates/for_each_type.h"
#include "augs/templates/type_list.h"

template <class F>
void for_each_component_type(F&& callback) {
	for_each_type_in_list<component_list_t<type_list>>(std::forward<F>(callback));
}

template <class F>
void for_each_invariant_type(F&& callback) {
	for_each_type_in_list<invariant_list_t<type_list>>(std::forward<F>(callback));
}