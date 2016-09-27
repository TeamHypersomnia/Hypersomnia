#include "game/transcendental/entity_handle.h"
#include "game/components/physics_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/transcendental/cosmos.h"
#include "spatial_properties_getters.h"

template <bool C, class D>
components::transform spatial_properties_getters<C, D>::logic_transform() const {
	auto& handle = *static_cast<const D*>(this);

	const auto& owner = handle.get_owner_body();

	if (owner.alive() && owner != handle) {
		return components::fixtures::transform_around_body(handle, owner.logic_transform());
	}
	else if (handle.has<components::physics>()) {
		ensure(!handle.has<components::transform>());
		const auto& phys = handle.get<components::physics>();
		return{ phys.get_position(), phys.get_angle() };
	}
	else {
		return handle.get<components::transform>();
	}
}

// explicit instantiation
template class spatial_properties_getters<false, basic_entity_handle<false>>;
template class spatial_properties_getters<true, basic_entity_handle<true>>;