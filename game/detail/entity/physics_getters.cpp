#include "game/entity_handle.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/cosmos.h"
#include "physics_getters.h"

template <class entity_handle_type>
entity_handle_type physics_getters<entity_handle_type>::get_owner_friction_field() const {
	auto& self = *static_cast<const entity_handle_type*>(this);
	return self.get_cosmos()[get_owner_body_entity().get<components::physics>().owner_friction_ground];
}

template <class entity_handle_type>
entity_handle_type physics_getters<entity_handle_type>::get_owner_body_entity() const {
	auto& self = *static_cast<const entity_handle_type*>(this);
	auto& cosmos = self.get_cosmos();

	auto* fixtures = self.find<components::fixtures>();
	if (fixtures) return cosmos[fixtures->get_body_entity()];
	else if (self.find<components::physics>()) return self;
	return cosmos[entity_id()];
}

// explicit instantiation
template class physics_getters<basic_entity_handle <false>>;
template class physics_getters<basic_entity_handle <true>>;