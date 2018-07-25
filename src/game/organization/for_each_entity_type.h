#pragma once
#include "augs/templates/for_each_type.h"
#include "augs/templates/type_list.h"

#include "game/cosmos/entity_solvable.h"

template <class F>
void for_each_entity_type(F&& callback) {
	for_each_type_in_list<all_entity_types>(std::forward<F>(callback));
}

template <class F>
void for_each_entity_solvable_type(F&& callback) {
	for_each_entity_type([&](auto e){
		using E = decltype(e);
		callback(entity_solvable<E>(), E());	
	});
}