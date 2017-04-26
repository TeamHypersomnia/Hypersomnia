#include "physics_mixin.h"
#include "game/assets/assets_manager.h"

#include "game/components/rigid_body_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

template <bool C, class D>
D basic_physics_mixin<C, D>::get_owner_friction_ground() const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[self.get_owner_body().get<components::special_physics>().owner_friction_ground];
}

// explicit instantiation
template class physics_mixin<false, basic_entity_handle<false>>;
template class physics_mixin<true, basic_entity_handle<true>>;
template class basic_physics_mixin<false, basic_entity_handle<false>>;
template class basic_physics_mixin<true, basic_entity_handle<true>>;