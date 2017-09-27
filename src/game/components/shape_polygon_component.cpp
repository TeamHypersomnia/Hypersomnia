#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/detail/physics/b2Fixture_index_in_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/inferential_systems/physics_system.h"

template <bool C>
bool basic_shape_polygon_synchronizer<C>::is_activated() const {
	return get_raw_component().activated;
}

using S = components::shape_polygon;

void component_synchronizer<false, S>::reinference() const {
	handle.get_cosmos().partial_reinference<physics_system>(handle);
}

convex_poly_destruction_data& component_synchronizer<false, S>::get_modifiable_destruction_data(
	const b2Fixture_index_in_component indices
) const {
	return get_raw_component().destruction[indices.convex_shape_index];
}

void component_synchronizer<false, S>::set_activated(const bool flag) const {
	if (flag == get_raw_component().activated) {
		return;
	}

	get_raw_component().activated = flag;
	reinference();
}

template class basic_shape_polygon_synchronizer<false>;
template class basic_shape_polygon_synchronizer<true>;