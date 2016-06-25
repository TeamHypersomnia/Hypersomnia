#include "game/entity_handle.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/cosmos.h"

template <bool C>
basic_entity_handle<C> basic_entity_handle<C>::get_owner_friction_field() const {
	return get_cosmos()[get_owner_body_entity().get<components::physics>().owner_friction_ground];
}

template <bool C>
basic_entity_handle<C> basic_entity_handle<C>::get_owner_body_entity() const {
	auto& cosmos = get_cosmos();

	auto* fixtures = find<components::fixtures>();
	if (fixtures) return cosmos[fixtures->get_body_entity()];
	else if (find<components::physics>()) return *this;
	return cosmos[entity_id()];
}

// explicit instantiation
template class basic_entity_handle <false>;
template class basic_entity_handle <true>;