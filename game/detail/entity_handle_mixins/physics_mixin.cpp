#include "game/transcendental/entity_handle.h"
#include "game/components/rigid_body_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/transcendental/cosmos.h"
#include "physics_mixin.h"

template <bool C, class D>
D physics_mixin<C, D>::get_owner_friction_ground() const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[self.get_owner_body().get<components::special_physics>().owner_friction_ground];
}

// explicit instantiation
template class physics_mixin<false, basic_entity_handle<false>>;
template class physics_mixin<true, basic_entity_handle<true>>;
