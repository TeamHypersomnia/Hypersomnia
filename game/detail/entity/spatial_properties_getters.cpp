#include "game/transcendental/entity_handle.h"
#include "game/components/physics_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/transcendental/cosmos.h"
#include "spatial_properties_getters.h"

template <bool C, class D>
bool basic_spatial_properties_getters<C, D>::has_logic_transform() const {
	auto& handle = *static_cast<const D*>(this);

	const auto& owner = handle.get_owner_body();
	
	if (owner.alive() && owner != handle) {
		return true;
	}
	else if (handle.has<components::physics>()) {
		return true;
	}
	else if (handle.has<components::transform>()) {
		return true;
	}

	return false;
}

template <bool C, class D>
components::transform basic_spatial_properties_getters<C, D>::logic_transform() const {
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

template <bool C, class D>
components::transform basic_spatial_properties_getters<C, D>::viewing_transform(const bool integerize) const {
	auto& handle = *static_cast<const D*>(this);
	return ::viewing_transform(handle, integerize);
}

template <class D>
void spatial_properties_getters<false, D>::set_logic_transform(const components::transform t) const {
	auto& handle = *static_cast<const D*>(this);

	const auto& owner = handle.get_owner_body();

	bool is_only_fixtural = owner.alive() && owner != handle;

	ensure(!is_only_fixtural);
	
	if (handle.has<components::physics>()) {
		ensure(!handle.has<components::transform>());
		const auto& phys = handle.get<components::physics>();
		phys.set_transform(t);
	}
	else {
		handle.get<components::transform>() = t;
	}
}

// explicit instantiation
template class spatial_properties_getters<false, basic_entity_handle<false>>;
template class spatial_properties_getters<true, basic_entity_handle<true>>;
template class basic_spatial_properties_getters<false, basic_entity_handle<false>>;
template class basic_spatial_properties_getters<true, basic_entity_handle<true>>;