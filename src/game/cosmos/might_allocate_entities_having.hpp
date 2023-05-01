#pragma once
#include "game/cosmos/cosmic_functions.h"
#include "game/organization/for_each_entity_type.h"

template <class... Types>
void cosmic::might_allocate_entities_having(cosmos& cosm, const std::size_t new_count) {
	for_each_entity_type([&](auto e) {
		using E = decltype(e);

		if constexpr(has_all_of_v<E, Types...>) {
			cosm.get_solvable({}).significant.template get_pool<E>().might_allocate_objects(new_count);
		}
	});
}

template <class... Types>
void cosmos::might_allocate_entities_having(const std::size_t new_count) {
	cosmic::might_allocate_entities_having<Types...>(*this, new_count);
}

inline void cosmos::might_allocate_stackable_entities(const std::size_t count) {
	might_allocate_entities_having<invariants::cartridge>(count);
}

template <class E>
bool cosmos::next_allocation_preserves_pointers() const {
	return get_solvable().significant.template get_pool<E>().next_allocation_preserves_pointers();
}
