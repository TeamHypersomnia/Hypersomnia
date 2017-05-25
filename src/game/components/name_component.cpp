#include "name_component.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/systems_inferred/name_system.h"
#include "game/components/all_inferred_state_component.h"

entity_id get_first_named_ancestor(const const_entity_handle p) {
	entity_id iterator = p;
	const auto& cosmos = p.get_cosmos();

	while (cosmos[iterator].alive()) {
		if (cosmos[iterator].has<components::name>()) {
			return iterator;
		}

		iterator = cosmos[iterator].get_parent();
	}

	return entity_id();
}

typedef components::name N;

template <bool C>
entity_name_id basic_name_synchronizer<C>::get_name_id() const {
	return get_data().name_id;
}

void component_synchronizer<false, N>::set_name_id(const entity_name_id& new_name_id) const {
	handle.get_cosmos().systems_inferred.get<name_system>().set_name_id(
		handle, 
		new_name_id
	);
}

template class basic_name_synchronizer<false>;
template class basic_name_synchronizer<true>;